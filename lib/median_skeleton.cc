/* Created on: Mar 4, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# include <median-path/median_skeleton.h>
# include <median-path/io.h>

# include <graphics-origin/tools/log.h>

BEGIN_MP_NAMESPACE

  static const uint8_t atom_links_property_index = 0;
  static const uint8_t atom_faces_property_index = 1;
  static const uint8_t link_faces_property_index = 0;

  /**TODO: When removing topologies, we should be sure the handles are still correct.
   * Inside the filter functions, we could remove topology as soon as we detect
   * an element to remove OR to mark topologies that should be removed and filter them
   * directly (might be more efficient).
   *
   *
   */


  void
  median_skeleton::remove_link_to_face( link_index idx, face_handle handle )
  {
    auto& face_elements = m_impl->m_link_properties[ link_faces_property_index ]->get<link_faces_property>( idx );
    auto size = face_elements.size();
    if( size <= 1 )
      link_faces_property().swap( face_elements );
    else
      {
        --size;
        for( size_t i = 0; i < size; ++ i )
          {
            if( face_elements[i].face == handle )
              {
                face_elements[i] = std::move( face_elements.back() );
                break;
              }
          }
        face_elements.pop_back();
      }
  }

  void
  median_skeleton::remove_atom_to_face( atom_index idx, face_handle handle )
  {
    auto& face_elements = m_impl->m_atom_properties[ atom_faces_property_index ]->get<atom_faces_property>( idx );
    auto size = face_elements.size();
    if( size <= 1 )
      atom_faces_property().swap( face_elements );
    else
      {
        --size;
        for( size_t i = 0; i < size; ++ i )
          {
            if( face_elements[i].face == handle )
              {
                face_elements[i] = std::move( face_elements.back() );
                break;
              }
          }
        face_elements.pop_back();
      }
  }

  void
  median_skeleton::remove_atom_to_link( atom_index idx, link_handle handle )
  {
    auto& link_elements = m_impl->m_atom_properties[ atom_links_property_index ]->get<atom_links_property>( idx );
    auto size = link_elements.size();
    if( size <= 1 )
      atom_links_property().swap( link_elements );
    else
      {
        --size;
        for( size_t i = 0; i < size; ++ i )
          {
            if( link_elements[i].first == handle )
              {
                link_elements[i] = std::move( link_elements.back() );
                break;
              }
          }
        link_elements.pop_back();
      }
  }

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
    : m_impl{ new datastructure{} }
  {
    m_impl->add_atom_property<atom_links_property>("links");
    m_impl->add_atom_property<atom_faces_property>("faces");
    m_impl->add_link_property<link_faces_property>("faces");

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
    return io::load( *this, filename );
  }

  bool median_skeleton::save( const std::string& filename )
  {
    return io::save( *this, filename );
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

  median_skeleton::atom_index median_skeleton::get_atoms_capacity() const noexcept
  {
    return m_impl->m_atoms_capacity;
  }
  median_skeleton::link_index median_skeleton::get_links_capacity() const noexcept
  {
    return m_impl->m_links_capacity;
  }
  median_skeleton::face_index median_skeleton::get_faces_capacity() const noexcept
  {
    return m_impl->m_faces_capacity;
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

  void
  median_skeleton::remove_atom_special_properties( atom_index idx )
  {
    auto& faces = m_impl->m_atom_properties[ atom_faces_property_index ]->get<atom_faces_property>( idx );
    // remove all the faces passing by this atom
    for( auto& fh : faces )
      {
        // remove the mapping atom -> face for the two other atoms
        for( int i = 0; i < 2; ++ i )
          {
            remove_atom_to_face(
                m_impl->m_atom_handles[ fh.atoms[i].index ].atom_index,
                fh.face );
          }
        // remove the mapping link -> face
        for( int i = 0; i < 3; ++ i )
          {
            remove_link_to_face(
                m_impl->m_link_handles[ fh.links[i].index ].link_index,
                fh.face );
          }
        // remove this face from the tight buffer
        m_impl->remove( fh.face ); // this removes all properties of the face
      }
    atom_faces_property().swap( faces ); // all mappings atom -> face for this atom are removed

    auto& links = m_impl->m_atom_properties[ atom_links_property_index ]->get<atom_links_property>( idx );
    // remove all the links having this atom as end point
    for( auto& lh : links )
      {
        // remove the mapping atom -> link for the other end point
        remove_atom_to_link(
            m_impl->m_atom_handles[ lh.second ].atom_index,
            lh.first );

        // if this link was part of a face, the mapping link -> face is already removed

        // remove this link from the tight buffer
        m_impl->remove( lh.first ); // this removes all other properties of the link
      }
    atom_links_property().swap( links );
  }

  void median_skeleton::remove( atom_handle handle )
  {
    if( handle.index < m_impl->m_atoms_capacity )
      {
        const auto entry = m_impl->m_atom_handles + handle.index;
        if( entry->status == datastructure::STATUS_ALLOCATED && entry->counter == handle.counter )
          {
            remove_atom_special_properties( entry->atom_index );
            m_impl->remove_atom_by_index( entry->atom_index );
          }
      }
  }

  void median_skeleton::remove( atom& e )
  {
    auto index = m_impl->get_index( e );
    remove_atom_special_properties( index );
    m_impl->remove_atom_by_index( index );
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

  median_skeleton::atom_index
  median_skeleton::get_index( atom& e ) const
  {
    return m_impl->get_index( e );
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
    /* When an atom is removed:
     *   all its links and faces should be removed
     *     - this is job of remove_link_indices(...), which guarantees
     *     that the tagged links no longer exist, either in the tight
     *     buffer or in topology properties. Since a face is destroyed
     *     once one of its links is removed, this is sufficient to call
     *     that function.
     *   it must be exchanged with the last atom A of the tight buffer
     *     - its handle will not be valid anymore
     *     - the handle of A need to point to the new memory
     *     - the index to handle index of A should be updated
     */


    const atom_index atom_size = m_impl->m_atoms_size;
    /* a flag buffer is necessary if the filter function relies on reference
     * to atoms or atom index. For example, if the filter function remove
     * atoms with indices 1 and 3, with 4 atoms in total in the buffer. When
     * atom #1 is removed, the atom #3 is move into it. Thus, when the former
     * atom #3 (now #1) is tested, the filter function will not recognize it. */
    bool* flags = new bool[ atom_size ];
    if( parallel )
      {
        # pragma omp parallel for schedule(dynamic)
        for( atom_index i = 0; i < atom_size; ++ i )
          {
            flags[ i ] = filter( m_impl->m_atoms[i] );
          }
      }
    else
      {
        for( atom_index i = 0; i < atom_size; ++ i )
          {
            flags[ i ] = filter( m_impl->m_atoms[i] );
          }
      }

    {
      const link_index link_size = m_impl->m_links_size;
      bool* link_flags = new bool[ link_size ];
      # pragma omp parallel for
      for( link_index i = 0; i < link_size; ++ i )
        link_flags[i] = false;
      # pragma omp parallel for
      for( atom_index i = 0; i < atom_size; ++ i )
        {
          if( flags[ i ] )
            {
              auto& alinks = m_impl->m_atom_properties[ atom_links_property_index ]->get<atom_links_property>( i );
              for( auto& alink : alinks )
                {
                  link_flags[ m_impl->m_link_handles[ alink.first.index ].link_index ] = true;
                }
            }
        }
      remove_link_indices(link_flags);
    }

    /* INVARIANT:
     * - all atoms with indices in [[0, left[[ are not removed
     * - all atoms with indices in [[left, right[[ need to be checked
     * - all atoms with indices in [[right, size[[ should be cleaned (all
     * objects referring to an atom should be destroyed)
     * Thus, in the end (i.e. when left == right ), we have:
     * - atoms with indices in [[0,left[[ are clean and tight
     * - atoms with indices in [[left, size[[ should be cleaned
     * - left is the new size of atoms
     */
    atom_index next_free_slot = m_impl->m_atoms_next_free_handle_slot;
    atom_index left = 0, right = atom_size;
    while( left < right )
      {
        if( flags[ left ] )
          {
            // update the left_entry
            const auto left_entry_index = m_impl->m_atom_index_to_handle_index[ left ];
            auto left_entry = m_impl->m_atom_handles + left_entry_index;
            left_entry->next_free_index = next_free_slot;
            left_entry->status = datastructure::STATUS_FREE;
            next_free_slot = left_entry_index;

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
                // move the last atom into this one to keep the buffer tight
                m_impl->m_atoms[left] = std::move( m_impl->m_atoms[right] );
                for( auto& property : m_impl->m_atom_properties )
                  property->move( right, left );

                const auto right_entry_index = m_impl->m_atom_index_to_handle_index[ right ];
                auto right_entry = m_impl->m_atom_handles + right_entry_index;
                right_entry->atom_index = left;
                m_impl->m_atom_index_to_handle_index[ left ] = right_entry_index;
                ++left;
              }
          }
        else ++ left;
      }
    delete[] flags;

    // destroy properties of atoms in [[left, size[[
    for( auto& property : m_impl->m_atom_properties )
      property->destroy( left, atom_size );

    m_impl->m_atoms_size = left;
    m_impl->m_atoms_next_free_handle_slot = next_free_slot;
  }

  median_skeleton::link_handle
  median_skeleton::add( atom_handle handle1, atom_handle handle2 )
  {
    if( handle1.index < m_impl->m_atoms_capacity && handle2.index < m_impl->m_atoms_capacity )
      {
        const auto entry1 = m_impl->m_atom_handles + handle1.index;
        const auto entry2 = m_impl->m_atom_handles + handle2.index;
        if(( entry1->status == datastructure::STATUS_ALLOCATED && entry1->counter == handle1.counter )
         &&( entry2->status == datastructure::STATUS_ALLOCATED && entry2->counter == handle2.counter ) )
          {
            return do_add_link( entry1->atom_index, entry2->atom_index );
          }
      }
    MP_THROW_EXCEPTION( skeleton_invalid_atom_handle );
  }

  median_skeleton::link_handle
  median_skeleton::add( atom& atom1, atom& atom2 )
  {
    if( &atom1 >= m_impl->m_atoms && &atom2 >= m_impl->m_atoms
     && &atom1 <  m_impl->m_atoms + m_impl->m_atoms_size && &atom2 < m_impl->m_atoms + m_impl->m_atoms_size )
      {
        atom_index idx1 = std::distance( m_impl->m_atoms, &atom1 );
        atom_index idx2 = std::distance( m_impl->m_atoms, &atom2 );
        if( m_impl->m_atoms + idx1 == &atom1 && m_impl->m_atoms + idx2 == &atom2 )
          {
            return do_add_link( idx1, idx2 );
          }
      }
    MP_THROW_EXCEPTION( skeleton_invalid_atom_handle );
  }

  median_skeleton::link_handle
  median_skeleton::add( atom_index idx1, atom_index idx2 )
  {
    if( idx1 < m_impl->m_atoms_size && idx2 < m_impl->m_atoms_size )
      {
        return do_add_link( idx1, idx2 );
      }
    MP_THROW_EXCEPTION( skeleton_invalid_atom_index );
  }

  median_skeleton::link_handle
  median_skeleton::do_add_link( atom_index idx1, atom_index idx2 )
  {
    auto entry_index2 = m_impl->m_atom_index_to_handle_index[ idx2 ];
    atom_handle handle2( entry_index2, m_impl->m_atom_handles[ entry_index2 ].counter );
    auto& links1 = m_impl->m_atom_properties[ atom_links_property_index ]->get<atom_links_property>( idx1 );
    // check for existing link
    for( auto& link : links1 )
      {
        if( link.second == handle2 )
          return link.first;
      }
    auto entry_index1 = m_impl->m_atom_index_to_handle_index[ idx1 ];
    atom_handle handle1( entry_index1, m_impl->m_atom_handles[ entry_index1 ].counter );
    auto& links2 = m_impl->m_atom_properties[ atom_links_property_index ]->get<atom_links_property>( idx2 );

    // set the link data
    auto result = m_impl->create_link();
    result.second.h1 = handle1;
    result.second.h2 = handle2;

    // set the link property for corresponding atoms
    links1.push_back( std::make_pair( result.first, handle2 ) );
    links2.push_back( std::make_pair( result.first, handle1 ) );
    return result.first;
  }

  void
  median_skeleton::remove( link_handle handle )
  {
    if( handle.index < m_impl->m_links_capacity )
      {
        const auto link_entry =  m_impl->m_link_handles + handle.index;
        if( link_entry->status == datastructure::STATUS_ALLOCATED && link_entry->counter == handle.counter )
          {
            const auto link_index = link_entry->link_index;
            auto& link = m_impl->m_links[ link_index ];
            atom_index idx1 = m_impl->m_atom_handles[ link.h1.index ].atom_index;
            atom_index idx2 = m_impl->m_atom_handles[ link.h2.index ].atom_index;

            // we start by destroying all the faces built with this link
            auto& faces = m_impl->m_link_properties[ link_faces_property_index ]->get<link_faces_property>( link_index );
            for( auto& face : faces )
              {
                // remove mapping link -> face from the two other links
                for( int i = 0; i < 2; ++ i )
                  {
                    remove_link_to_face(
                        m_impl->m_link_handles[ face.links[i].index ].link_index,
                        face.face );
                  }
                // remove mapping atom -> face from the three atoms
                remove_atom_to_face( idx1, face.face );
                remove_atom_to_face( idx2, face.face );
                remove_atom_to_face(
                    m_impl->m_atom_handles[ face.opposite.index ].atom_index,
                    face.face );
                // remove the face from the tight buffer
                m_impl->remove( face.face ); // this removes all other properties of that face
              }
            link_faces_property().swap( faces ); // all mappings link -> face for this link are removed

            // remove that link
            remove_atom_to_link( idx1, handle );
            remove_atom_to_link( idx2, handle );
            m_impl->remove( handle ); // this removes all other properties of that link
          }
      }
  }

  void
  median_skeleton::remove( link& e )
  {
    auto link_index = m_impl->get_index( e );

    atom_index idx1 = m_impl->m_atom_handles[ e.h1.index ].atom_index;
    atom_index idx2 = m_impl->m_atom_handles[ e.h2.index ].atom_index;

    // we start by destroying all the faces built with this link
    auto& faces = m_impl->m_link_properties[ link_faces_property_index ]->get<link_faces_property>( link_index );
    for( auto& face : faces )
      {
        // remove mapping link -> face from the two other links
        for( int i = 0; i < 2; ++ i )
          {
            remove_link_to_face(
                m_impl->m_link_handles[ face.links[i].index ].link_index,
                face.face );
          }
        // remove mapping atom -> face from the three atoms
        remove_atom_to_face( idx1, face.face );
        remove_atom_to_face( idx2, face.face );
        remove_atom_to_face(
            m_impl->m_atom_handles[ face.opposite.index ].atom_index,
            face.face );
        // remove the face from the tight buffer
        m_impl->remove( face.face ); // this removes all other properties of that face
      }
    link_faces_property().swap( faces ); // all mappings link -> face for this link are removed

    // remove that link
    auto handle = link_handle( m_impl->m_link_index_to_handle_index[ link_index ], 0 );
    handle.counter = m_impl->m_link_handles[ handle.index ].counter;
    remove_atom_to_link( idx1, handle );
    remove_atom_to_link( idx2, handle );
    m_impl->remove( e ); // this removes all other properties of that link
  }

  median_skeleton::link&
  median_skeleton::get( link_handle handle ) const
  {
    return m_impl->get( handle );
  }

  median_skeleton::link&
  median_skeleton::get_link_by_index( link_index index ) const
  {
    return m_impl->get_link_by_index( index );
  }

  median_skeleton::link_index
  median_skeleton::get_index( link_handle handle ) const
  {
    return m_impl->get_index( handle );
  }

  median_skeleton::link_index
  median_skeleton::get_index( link& e ) const
  {
    return m_impl->get_index( e );
  }

  median_skeleton::link_handle
  median_skeleton::get_handle( link& e ) const
  {
    return m_impl->get_handle( e );
  }

  bool
  median_skeleton::is_valid( link_handle handle ) const
  {
    if( handle.index < m_impl->m_links_capacity )
      {
        const auto entry = m_impl->m_link_handles + handle.index;
        return entry->status == datastructure::STATUS_ALLOCATED && entry->counter == handle.counter;
      }
    return false;
  }

  void
  median_skeleton::process( link_processer&& function, bool parallel )
  {
    // TODO: use arguments in the pragma to use or not threads
    if( parallel )
      {
        # pragma omp parallel for schedule(dynamic)
        for( link_index i = 0; i < m_impl->m_links_size; ++ i )
          {
            function( m_impl->m_links[i]);
          }
      }
    else
      {
        for( link_index i = 0; i < m_impl->m_links_size; ++ i )
          {
            function( m_impl->m_links[i]);
          }
      }
  }


  void
  median_skeleton::remove( link_filter&& filter, bool parallel )
  {
    const link_index size = m_impl->m_links_size;
    /* a flag buffer is necessary if the filter function relies on reference
     * to links or link index. For example, if the filter function remove
     * links with indices 1 and 3, with 4 links in total in the buffer. When
     * link #1 is removed, the link #3 is move into it. Thus, when the former
     * link #3 (now #1) is tested, the filter function will not recognize it. */
    bool* flags = new bool[ size ];
    if( parallel )
      {
        # pragma omp parallel for schedule(dynamic)
        for( link_index i = 0; i < size; ++ i )
          {
            flags[ i ] = filter( m_impl->m_links[i] );
          }
      }
    else
      {
        for( link_index i = 0; i < size; ++ i )
          {
            flags[ i ] = filter( m_impl->m_links[i] );
          }
      }

    remove_link_indices( flags );
  }

  void
  median_skeleton::remove_link_indices( bool* flags )
  {
    /* When a link is removed:
     *   all its faces should be removed
     *     - the removal will update the atom_face_property and link_face_property
     *     - the removal will change the face indices
     *   its two atoms should be notified they are no longer connected
     *     - the connection is identified by the link handle
     *     - thus this notification should be done before modifying the link buffer
     *   it must be exchanged with the last link l of the tight buffer
     *     - its handle wont be valid anymore
     *     - the handle of l need to point to the new memory
     *     - the index to handle index of l should be updated
     */


    const link_index link_size = m_impl->m_links_size;
    /* For each link to remove:
     *  - all faces connected are tagged, then removed
     *  - the two atoms are notified they are no longer connected
     */
    {
      const face_index face_size = m_impl->m_faces_size;
      bool* face_flags = new bool[ face_size ];
      # pragma omp parallel for
      for( face_index i = 0; i < face_size; ++ i )
        face_flags[i] = false;

      for( link_index i = 0; i < link_size; ++ i )
        {
          if( flags[i] )
            {
              auto& lfaces = m_impl->m_link_properties[ link_faces_property_index ]->get<link_faces_property>( i );
              for( auto& lface : lfaces )
                {
                  face_flags[ m_impl->m_face_handles[ lface.face.index ].face_index ] = true;
                }

              auto& link = m_impl->m_links[i];
              link_handle lhandle( m_impl->m_link_index_to_handle_index[ i ], 0 );
              lhandle.counter = m_impl->m_link_handles[ lhandle.index ].counter;

              remove_atom_to_link( m_impl->m_atom_handles[ link.h1.index ].atom_index, lhandle );
              remove_atom_to_link( m_impl->m_atom_handles[ link.h2.index ].atom_index, lhandle );
            }
        }
      // this will deal with the atom_face_property and link_face_property
      remove_faces_indices( face_flags );
    }

    /* INVARIANT:
     * - all links with indices in [[0, left[[ are not removed
     * - all links with indices in [[left, right[[ need to be checked
     * - all links with indices in [[right, size[[ should have their properties cleaned
     *
     * Thus, in the end (i.e. when left == right ), we have:
     * - links with indices in [[0,left[[ are clean and tight
     * - links with indices in [[left, size[[ should have their properties cleaned
     * - left is the new size of links */
    link_index next_free_slot = m_impl->m_links_next_free_handle_slot;
    link_index left = 0, right = link_size;
    while( left < right )
      {
        if( flags[ left ] )
          {
            // update the left_entry
            const auto left_entry_index = m_impl->m_link_index_to_handle_index[ left ];
            auto left_entry = m_impl->m_link_handles + left_entry_index;
            left_entry->next_free_index = next_free_slot;
            left_entry->status = datastructure::STATUS_FREE;
            next_free_slot = left_entry_index;

            // look for a valid link in the end of the buffer
            --right;
            while( left < right && flags[ right ] )
              {
                const auto entry_index = m_impl->m_link_index_to_handle_index[ right ];
                auto entry = m_impl->m_link_handles + entry_index;
                entry->next_free_index = next_free_slot;
                entry->status = datastructure::STATUS_FREE;
                next_free_slot = entry_index;
                --right;
              }

            if( left != right )
              {
                // move the last link into this one to keep the buffer tight
                m_impl->m_links[left] = std::move( m_impl->m_links[right] );
                for( auto& property : m_impl->m_link_properties )
                  property->move( right, left );

                const auto right_entry_index = m_impl->m_link_index_to_handle_index[ right ];
                auto right_entry = m_impl->m_link_handles + right_entry_index;
                right_entry->link_index = left;
                m_impl->m_link_index_to_handle_index[ left ] = right_entry_index;
                ++left;
              }
          }
        else ++ left;
      }
    delete[] flags;

    // destroy properties of links in [[left, size[[
    for( auto& property : m_impl->m_link_properties )
      property->destroy( left, link_size );

    m_impl->m_links_size = left;
    m_impl->m_links_next_free_handle_slot = next_free_slot;
  }

  median_skeleton::face_handle
  median_skeleton::add( atom& atom1, atom& atom2, atom& atom3 )
  {
    if( &atom1 >= m_impl->m_atoms && &atom2 >= m_impl->m_atoms && &atom3 >= m_impl->m_atoms
     && &atom1 < m_impl->m_atoms + m_impl->m_atoms_size
     && &atom2 < m_impl->m_atoms + m_impl->m_atoms_size
     && &atom3 < m_impl->m_atoms + m_impl->m_atoms_size )
      {
        atom_index idx1 = std::distance( m_impl->m_atoms, &atom1 );
        atom_index idx2 = std::distance( m_impl->m_atoms, &atom2 );
        atom_index idx3 = std::distance( m_impl->m_atoms, &atom3 );
        if( m_impl->m_atoms + idx1 == &atom1 && m_impl->m_atoms + idx2 == &atom2 && m_impl->m_atoms + idx3 == &atom3 )
          {
            return do_add_face( idx1, idx2, idx3 );
          }
      }
    MP_THROW_EXCEPTION( skeleton_invalid_atom_pointer );
  }

  median_skeleton::face_handle
  median_skeleton::add( atom_index idx1, atom_index idx2, atom_index idx3 )
  {
    if( idx1 < m_impl->m_atoms_size && idx2 < m_impl->m_atoms_size && idx3 < m_impl->m_atoms_size )
      {
        return do_add_face( idx1, idx2, idx3 );
      }
    MP_THROW_EXCEPTION( skeleton_invalid_atom_index );
  }

  median_skeleton::face_handle
  median_skeleton::do_add_face( atom_index idx1, atom_index idx2, atom_index idx3 )
  {
    auto& faces1 = m_impl->m_atom_properties[ atom_faces_property_index ]->get< atom_faces_property >( idx1 );
    for( auto& face : faces1 )
      {
        if( ( face.atoms[0] == idx2 && face.atoms[1] == idx3 )
         && ( face.atoms[0] == idx3 && face.atoms[1] == idx2 ) )
          {
            return face.face;
          }
      }

    auto result = m_impl->create_face();

    result.second.atoms[0].index = m_impl->m_atom_index_to_handle_index[ idx1 ];
    result.second.atoms[0].counter = m_impl->m_atom_handles[ result.second.atoms[0].index ].counter;
    result.second.atoms[1].index = m_impl->m_atom_index_to_handle_index[ idx2 ];
    result.second.atoms[1].counter = m_impl->m_atom_handles[ result.second.atoms[1].index ].counter;
    result.second.atoms[2].index = m_impl->m_atom_index_to_handle_index[ idx3 ];
    result.second.atoms[2].counter = m_impl->m_atom_handles[ result.second.atoms[2].index ].counter;

    // fetch the face links, creating them if needed
    {
      auto& links1 = m_impl->m_atom_properties[ atom_links_property_index ]->get<atom_links_property>( idx1 );
      auto& links2 = m_impl->m_atom_properties[ atom_links_property_index ]->get<atom_links_property>( idx2 );
      auto& links3 = m_impl->m_atom_properties[ atom_links_property_index ]->get<atom_links_property>( idx3 );

      // search for link 1 -- 2
      for( auto& link : links1 )
        {
          if( link.second == result.second.atoms[1] )
            {
              result.second.links[0] = link.first;
              break;
            }
        }
      // create the link 1 -- 2 if it does not exist
      if( !result.second.links[0].is_valid() )
        {
          auto pair = m_impl->create_link();

          pair.second.h1 = result.second.atoms[0];
          pair.second.h2 = result.second.atoms[1];

          links1.push_back( std::make_pair( pair.first, pair.second.h2 ));
          links2.push_back( std::make_pair( pair.first, pair.second.h1 ));

          result.second.links[0] = pair.first;
        }

      // search for link 2 -- 3
      for( auto& link : links2 )
        {
          if( link.second == result.second.atoms[2] )
            {
              result.second.links[1] = link.first;
              break;
            }
        }
      // create the link 2 -- 3 if it does not exist
      if( !result.second.links[1].is_valid() )
        {
          auto pair = m_impl->create_link();

          pair.second.h1 = result.second.atoms[1];
          pair.second.h2 = result.second.atoms[2];

          links2.push_back( std::make_pair( pair.first, pair.second.h2 ));
          links3.push_back( std::make_pair( pair.first, pair.second.h1 ));

          result.second.links[1] = pair.first;
        }

      // search for the link 3 -- 1
      for( auto& link : links3 )
        {
          if( link.second == result.second.atoms[0] )
            {
              result.second.links[2] = link.first;
              break;
            }
        }
      // create the link 0 -- 1 if it does not exist
      if( !result.second.links[2].is_valid() )
        {
          auto pair = m_impl->create_link();

          pair.second.h1 = result.second.atoms[2];
          pair.second.h2 = result.second.atoms[0];

          links3.push_back( std::make_pair( pair.first, pair.second.h2 ));
          links1.push_back( std::make_pair( pair.first, pair.second.h1 ));

          result.second.links[2] = pair.first;
        }
    }

    // set the atom face elements
    {
      m_impl->m_atom_properties[ atom_faces_property_index ]
         ->get< atom_faces_property >( idx1 ).push_back(
            atom_face_element( result.second, result.first, 0 )
      );
      m_impl->m_atom_properties[ atom_faces_property_index ]
         ->get< atom_faces_property >( idx2 ).push_back(
            atom_face_element( result.second, result.first, 1 )
      );
      m_impl->m_atom_properties[ atom_faces_property_index ]
         ->get< atom_faces_property >( idx3 ).push_back(
            atom_face_element( result.second, result.first, 2 )
      );
    }

    // set the link face elements
    m_impl->m_link_properties[link_faces_property_index]
      ->get< link_faces_property >(
        m_impl->m_link_handles[result.second.links[0].index].link_index
      ).push_back (
        link_face_element (result.second, result.first, 0)
    );
    m_impl->m_link_properties[link_faces_property_index]
      ->get< link_faces_property >(
        m_impl->m_link_handles[result.second.links[1].index].link_index
      ).push_back (
        link_face_element (result.second, result.first, 1)
    );
    m_impl->m_link_properties[link_faces_property_index]
      ->get< link_faces_property >(
        m_impl->m_link_handles[result.second.links[2].index].link_index
      ).push_back (
        link_face_element (result.second, result.first, 2)
    );

    return result.first;
  }

  median_skeleton::face_handle
  median_skeleton::add( atom_handle handle1, atom_handle handle2, atom_handle handle3 )
  {
    if( handle1.index < m_impl->m_atoms_capacity
        && handle2.index < m_impl->m_atoms_capacity
        && handle3.index < m_impl->m_atoms_capacity )
      {
        const auto entry1 = m_impl->m_atom_handles + handle1.index;
        const auto entry2 = m_impl->m_atom_handles + handle2.index;
        const auto entry3 = m_impl->m_atom_handles + handle3.index;
        if(( entry1->status == datastructure::STATUS_ALLOCATED && entry1->counter == handle1.counter )
         &&( entry2->status == datastructure::STATUS_ALLOCATED && entry2->counter == handle2.counter )
         &&( entry3->status == datastructure::STATUS_ALLOCATED && entry3->counter == handle3.counter ))
          {
            return do_add_face( entry1->atom_index, entry2->atom_index, entry3->atom_index );
          }
      }
    MP_THROW_EXCEPTION( skeleton_invalid_atom_handle );
  }

  void
  median_skeleton::remove( face_handle handle )
  {
    if( handle.index < m_impl->m_faces_capacity )
      {
        const auto face_entry =  m_impl->m_face_handles + handle.index;
        if( face_entry->status == datastructure::STATUS_ALLOCATED && face_entry->counter == handle.counter )
          {
            remove_face_topology_properties( face_entry->face_index, handle );
            m_impl->remove_face_by_index( face_entry->face_index );
          }
      }
  }

  void
  median_skeleton::remove( face& e )
  {
    auto index = get_index( e );
    auto handle_index = m_impl->m_face_index_to_handle_index[ index ];
    face_handle handle( handle_index, m_impl->m_face_handles[ handle_index ].counter );
    remove_face_topology_properties( index, handle );
    m_impl->remove_face_by_index( index );
  }

  void
  median_skeleton::remove_face_topology_properties( face_index idx, face_handle handle )
  {
    auto& face = m_impl->m_faces[ idx ];

    remove_atom_to_face( m_impl->m_atom_handles[ face.atoms[0].index ].atom_index, handle );
    remove_atom_to_face( m_impl->m_atom_handles[ face.atoms[1].index ].atom_index, handle );
    remove_atom_to_face( m_impl->m_atom_handles[ face.atoms[2].index ].atom_index, handle );

    remove_link_to_face( m_impl->m_link_handles[ face.links[0].index ].link_index, handle );
    remove_link_to_face( m_impl->m_link_handles[ face.links[1].index ].link_index, handle );
    remove_link_to_face( m_impl->m_link_handles[ face.links[2].index ].link_index, handle );
  }

  median_skeleton::face&
  median_skeleton::get( face_handle handle ) const
  {
    return m_impl->get( handle );
  }

  median_skeleton::face&
  median_skeleton::get_face_by_index( face_index index ) const
  {
    return m_impl->get_face_by_index( index );
  }

  median_skeleton::face_index
  median_skeleton::get_index( face_handle handle ) const
  {
    return m_impl->get_index( handle );
  }

  median_skeleton::face_index
  median_skeleton::get_index( face& e ) const
  {
    return m_impl->get_index( e );
  }

  median_skeleton::face_handle
  median_skeleton::get_handle( face& e ) const
  {
    return m_impl->get_handle( e );
  }

  bool
  median_skeleton::is_valid( face_handle handle ) const
  {
    if( handle.index < m_impl->m_faces_capacity )
      {
        const auto entry = m_impl->m_face_handles + handle.index;
        return entry->status == datastructure::STATUS_ALLOCATED && entry->counter == handle.counter;
      }
    return false;
  }

  void
  median_skeleton::process( face_processer&& function, bool parallel )
  {
    // TODO: use arguments in the pragma to use or not threads
    if( parallel )
      {
        # pragma omp parallel for schedule(dynamic)
        for( face_index i = 0; i < m_impl->m_faces_size; ++ i )
          {
            function( m_impl->m_faces[i]);
          }
      }
    else
      {
        for( face_index i = 0; i < m_impl->m_faces_size; ++ i )
          {
            function( m_impl->m_faces[i]);
          }
      }
  }

  void
  median_skeleton::remove_faces_indices( bool* flags )
  {
    /* INVARIANT:
     * - all faces with indices in [[0, left[[ are not removed
     * - all faces with indices in [[left, right[[ need to be checked
     * - all face properties in [[right, size[[ should be cleaned
     *
     * When a deleted face is detected, its references in atom and link
     * properties are immediately removed. Indeed, such removals require
     * to know the face handle and the face itself. If deleted faces are
     * put in the end of the tight buffer, handles and faces need to be
     * updated such that we could perform the removal in the end of the function.
     *
     * Thus, in the end (i.e. when left == right ), we have:
     * - faces with indices in [[0,left[[ are clean and tight
     * - face properties in [[left, size[[ should be cleaned
     * - left is the new size of faces
     */

    const face_index size = m_impl->m_faces_size;
    face_index next_free_slot = m_impl->m_faces_next_free_handle_slot;
    face_index left = 0, right = size;
    while( left < right )
      {
        if( flags[ left ] )
          {
            // update the left_entry
            const auto left_entry_index = m_impl->m_face_index_to_handle_index[ left ];
            auto left_entry = m_impl->m_face_handles + left_entry_index;
            remove_face_topology_properties( left, face_handle( left_entry_index, left_entry->counter ) );
            left_entry->next_free_index = next_free_slot;
            left_entry->status = datastructure::STATUS_FREE;
            next_free_slot = left_entry_index;

            // look for a valid face in the end of the buffer
            --right;
            while( left < right && flags[ right ] )
              {
                const auto entry_index = m_impl->m_face_index_to_handle_index[ right ];
                auto entry = m_impl->m_face_handles + entry_index;
                remove_face_topology_properties( right, face_handle( entry_index, entry->counter ) );
                entry->next_free_index = next_free_slot;
                entry->status = datastructure::STATUS_FREE;
                next_free_slot = entry_index;
                --right;
              }

            if( left != right )
              {
                // move the last face into this one to keep the buffer tight
                m_impl->m_faces[left] = std::move( m_impl->m_faces[right] );
                for( auto& property : m_impl->m_face_properties )
                  property->move( right, left );

                const auto right_entry_index = m_impl->m_face_index_to_handle_index[ right ];
                auto right_entry = m_impl->m_face_handles + right_entry_index;
                right_entry->face_index = left;
                m_impl->m_face_index_to_handle_index[ left ] = right_entry_index;
                ++left;
              }
          }
        else ++ left;
      }
    delete[] flags;

    // cleaning face properties in [[left, size[[
    for( auto& property : m_impl->m_face_properties )
      property->destroy( left, size );

    m_impl->m_faces_size = left;
    m_impl->m_faces_next_free_handle_slot = next_free_slot;
  }


  void
  median_skeleton::remove( face_filter&& filter, bool parallel )
  {
    const face_index size = m_impl->m_faces_size;
    /* a flag buffer is necessary if the filter function relies on reference
     * to faces or face index. For example, if the filter function remove
     * faces with indices 1 and 3, with 4 faces in total in the buffer. When
     * face #1 is removed, the face #3 is move into it. Thus, when the former
     * face #3 (now #1) is tested, the filter function will not recognize it. */
    bool* flags = new bool[ size ];
    if( parallel )
      {
        # pragma omp parallel for schedule(dynamic)
        for( face_index i = 0; i < size; ++ i )
          {
            flags[ i ] = filter( m_impl->m_faces[i] );
          }
      }
    else
      {
        for( face_index i = 0; i < size; ++ i )
          {
            flags[ i ] = filter( m_impl->m_faces[i] );
          }
      }

    remove_faces_indices( flags );
  }

END_MP_NAMESPACE
