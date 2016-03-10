/* Created on: Mar 4, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# include <median-path/median_skeleton.h>
# include <graphics-origin/tools/log.h>

# include <thrust/scan.h>

BEGIN_MP_NAMESPACE

  static const uint8_t atom_links_property_index = 0;
  static const uint8_t atom_faces_property_index = 1;
  static const uint8_t link_faces_property_index = 0;

  median_skeleton::median_skeleton(
      atom_index atom_capacity,
      link_index link_capacity,
      face_index face_capacity)
    : m_impl{ new datastructure{ atom_capacity, link_capacity, face_capacity } }
  {
    m_impl->add_atom_property<atom_links_property>("links");
    m_impl->add_atom_property<atom_faces_property>("faces");
    m_impl->add_link_property<link_faces_property>("faces");
  }

  median_skeleton::median_skeleton( median_skeleton&& other )
    : m_impl{ other.m_impl }
  {
    other.m_impl = new datastructure{};
  }

  median_skeleton::median_skeleton( const std::string& filename )
  {
    load( filename );
  }

  median_skeleton::~median_skeleton()
  {
    delete m_impl;
  }

  void median_skeleton::clear(
      atom_index atom_capacity,
      uint64_t link_capacity,
      uint64_t face_capacity )
  {
    m_impl->clear( atom_capacity, link_capacity, face_capacity );
  }

  bool median_skeleton::load( const std::string& filename )
  {
    LOG( debug, "TODO: load a file not implemented");
    return false;
  }

  bool median_skeleton::save( const std::string& filename )
  {
    LOG( debug, "TODO: save a file not implemented");
    return false;
  }

  median_skeleton::atom_index median_skeleton::get_number_of_atoms() const noexcept
  {
    return m_impl->m_atoms_size;
  }
  uint64_t median_skeleton::get_number_of_links() const noexcept
  {
    return m_impl->m_links_size;
  }
  uint64_t median_skeleton::get_number_of_faces() const noexcept
  {
    return m_impl->m_faces_size;
  }

  median_skeleton::atom_handle
  median_skeleton::add( const vec3& position, const real& radius )
  {
    auto pair = m_impl->create_atom();
    pair.second = atom{ position, radius };
    return pair.first;
  }

  median_skeleton::atom_handle
  median_skeleton::add( const vec4& ball )
  {
    auto pair = m_impl->create_atom();
    pair.second = ball;
    return pair.first;
  }

  void median_skeleton::remove( atom_handle handle )
  {
    if( handle.index < m_impl->m_atoms_capacity )
      {
        const auto entry = m_impl->m_atom_handles + handle.index;
        if( entry->status == datastructure::STATUS_ALLOCATED && entry->counter == handle.counter )
          {
            auto index = entry->atom_index;
            auto& links = m_impl->m_atom_properties[ atom_links_property_index ]->get<atom_links_property>( index );
            auto& faces = m_impl->m_atom_properties[ atom_faces_property_index ]->get<atom_faces_property>( index );

            // a link can have no face
            // a face must have three links
            // if we remove the faces first, most of the links would be removed too.
            for( auto& fh : faces )
              m_impl->remove( fh );
            for( auto& lh : links )
              m_impl->remove( lh );
            // properties are automatically removed during this call:
            m_impl->remove( handle );
          }
      }
  }

  void median_skeleton::remove( atom& e )
  {
    auto index = m_impl->get_index( e );
    auto& links = m_impl->m_atom_properties[ atom_links_property_index ]->get<atom_links_property>( index );
    auto& faces = m_impl->m_atom_properties[ atom_faces_property_index ]->get<atom_faces_property>( index );

    // a link can have no face
    // a face must have three links
    // if we remove the faces first, most of the links would be removed too.
    for( auto& fh : faces )
      m_impl->remove( fh );
    for( auto& lh : links )
      m_impl->remove( lh );
    // properties are automatically removed during this call:
    m_impl->remove( e );
  }

  median_skeleton::atom&
  median_skeleton::get( atom_handle handle ) const
  {
    return m_impl->get(handle);
  }

  median_skeleton::atom&
  median_skeleton::get_atom_by_index( atom_index index ) const
  {
    return m_impl->get_atom_by_index( index );
  }

  median_skeleton::atom_index
  median_skeleton::get_index( atom_handle handle ) const
  {
    return m_impl->get_index( handle );
  }

  median_skeleton::atom_handle
  median_skeleton::get_handle( atom&e ) const
  {
    return m_impl->get_handle( e );
  }

  bool
  median_skeleton::is_valid( atom_handle handle ) const
  {
    if( handle.index < m_impl->m_atoms_capacity )
      {
        const auto entry = m_impl->m_atom_handles + handle.index;
        return entry->status == datastructure::STATUS_ALLOCATED && entry->counter == handle.counter;
      }
    return false;
  }

  void
  median_skeleton::process( atom_processer&& function, bool parallel )
  {
    // TODO: use arguments in the pragma to use or not threads
    if( parallel )
      {
        # pragma omp parallel for schedule(dynamic)
        for( atom_index i = 0; i < m_impl->m_atoms_size; ++ i )
          {
            function( m_impl->m_atoms[i]);
          }
      }
    else
      {
        for( atom_index i = 0; i < m_impl->m_atoms_size; ++ i )
          {
            function( m_impl->m_atoms[i]);
          }
      }
  }

  void
  median_skeleton::remove( atom_filter&& filter, bool parallel )
  {
    const atom_index size = m_impl->m_atoms_size;
    /* a flag buffer is necessary if the filter function relies on reference
     * to atoms or atom index. For example, if the filter function remove
     * atoms with indices 1 and 3, with 4 atoms in total in the buffer. When
     * atom #1 is removed, the atom #3 is move into it. Thus, when the former
     * atom #3 (now #1) is tested, the filter function will not recognize it. */
    bool* flags = new bool[ size ];
    if( parallel )
      {
        # pragma omp parallel for schedule(dynamic)
        for( atom_index i = 0; i < size; ++ i )
          {
            flags[ i ] = filter( m_impl->m_atoms[i] );
          }
      }
    else
      {
        for( atom_index i = 0; i < size; ++ i )
          {
            flags[ i ] = filter( m_impl->m_atoms[i] );
          }
      }

    /* INVARIANT:
     *   all atoms with indices in [[0, left[[ are not removed
     *   all atoms with indices in [[left, right[[ need to be checked
     *   all atoms with indices in [[right, size[[ should have their properties destroyed
     * Thus, in the end (i.e. when left == right ), we have:
     *   atoms with indices in [[0,left[[ are clean and tight
     *   atoms with indices in [[left, size[[ should have their properties destroyed
     *   left is the new size of atoms
     */
    atom_index next_free_slot = m_impl->m_atoms_next_free_handle_slot;
    atom_index left = 0, right = size;
    while( left < right )
      {
        if( flags[ left ] )
          {
            // look for a valid atom in the end of the buffer
            --right;
            while( left < right && flags[ right ] )
              {
                const auto entry_index = m_impl->m_atom_index_to_handle_index[ right ];
                auto entry = m_impl->m_atom_handles + entry_index;
                entry->next_free_index = next_free_slot;
                entry->status = datastructure::STATUS_FREE;
                next_free_slot = entry_index;
                --right;
              }

            if( left != right )
              {
                // update the left_entry
                const auto left_entry_index = m_impl->m_atom_index_to_handle_index[ left ];
                auto left_entry = m_impl->m_atom_handles + left_entry_index;
                left_entry->next_free_index = next_free_slot;
                left_entry->status = datastructure::STATUS_FREE;
                next_free_slot = left_entry_index;

                // move the last atom into this one to keep the buffer tight
                m_impl->m_atoms[left] = std::move( m_impl->m_atoms[right] );
                for( auto& property : m_impl->m_atom_properties )
                  property->move( right, left );

                const auto right_entry_index = m_impl->m_atom_index_to_handle_index[ right ];
                auto right_entry = m_impl->m_atom_handles + right_entry_index;
                right_entry->atom_index = left;
                m_impl->m_atom_index_to_handle_index[ left ] = right_entry_index;
              }
          }
        else ++ left;
      }
    delete[] flags;

    // destroy properties of atoms in [[left, size[[
    for( auto& property : m_impl->m_atom_properties )
      property->destroy( left, size );

    m_impl->m_atoms_size = left;
    m_impl->m_atoms_next_free_handle_slot = next_free_slot;
  }



END_MP_NAMESPACE
