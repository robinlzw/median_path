/* Created on: Mar 4, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# include <median-path/median_skeleton.h>
# include <graphics-origin/tools/log.h>

BEGIN_MP_NAMESPACE

  static const uint8_t atom_links_property_index = 0;
  static const uint8_t atom_faces_property_index = 1;
  static const uint8_t link_faces_property_index = 0;

  median_skeleton::median_skeleton(
      uint32_t atom_capacity,
      uint64_t link_capacity,
      uint64_t face_capacity)
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
      uint32_t atom_capacity,
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

  uint32_t median_skeleton::get_number_of_atoms() const noexcept
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
  median_skeleton::get_atom_by_index( uint32_t index ) const
  {
    return m_impl->get_atom_by_index( index );
  }

  uint32_t
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


END_MP_NAMESPACE
