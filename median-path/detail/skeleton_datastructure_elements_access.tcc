  dts_definition(typename dts_type::atom&)::get( atom_handle h )
  {
# ifndef MP_SKELETON_NO_HANDLE_CHECK
    if( h.index >= m_atoms_capacity )
      MP_THROW_EXCEPTION( skeleton_invalid_atom_handle );
# endif
    const auto entry = m_atom_handles.get() + h.index;
# ifndef MP_SKELETON_NO_HANDLE_CHECK
    if( entry->status != STATUS_ALLOCATED || entry->counter != h.counter )
      MP_THROW_EXCEPTION( skeleton_invalid_atom_handle );
# endif
    return m_atoms[ entry->atom_index ];
  }


  dts_definition(typename dts_type::link&)::get( link_handle h )
  {
# ifndef MP_SKELETON_NO_HANDLE_CHECK
    if( h.index >= m_links_capacity )
      MP_THROW_EXCEPTION( skeleton_invalid_link_handle );
# endif
    const auto entry = m_link_handles.get() + h.index;
# ifndef MP_SKELETON_NO_HANDLE_CHECK
    if( entry->status != STATUS_ALLOCATED || entry->counter != h.counter )
      MP_THROW_EXCEPTION( skeleton_invalid_link_handle );
# endif
    return m_links[ entry->link_index ];
  }

  dts_definition(typename dts_type::face&)::get( face_handle h )
  {
# ifndef MP_SKELETON_NO_HANDLE_CHECK
    if( h.index >= m_faces_capacity )
      MP_THROW_EXCEPTION( skeleton_invalid_face_handle );
# endif
    const auto entry = m_face_handles.get() + h.index;
# ifndef MP_SKELETON_NO_HANDLE_CHECK
    if( entry->status != STATUS_ALLOCATED || entry->counter != h.counter )
      MP_THROW_EXCEPTION( skeleton_invalid_face_handle );
# endif
    return m_faces[ entry->face_index ];
  }

  dts_definition(typename dts_type::atom_index)::get_index( atom_handle handle )
  {
# ifndef MP_SKELETON_NO_HANDLE_CHECK
    if( handle.index >= m_atoms_capacity )
      MP_THROW_EXCEPTION( skeleton_invalid_atom_handle );
# endif
    const auto entry = m_atom_handles.get() + handle.index;
# ifndef MP_SKELETON_NO_HANDLE_CHECK
    if( entry->status != STATUS_ALLOCATED || entry->counter != handle.counter )
      MP_THROW_EXCEPTION( skeleton_invalid_atom_handle );
# endif
    return entry->atom_index;
  }

  dts_definition(typename dts_type::link_index)::get_index( link_handle handle )
  {
# ifndef MP_SKELETON_NO_HANDLE_CHECK
    if( handle.index >= m_links_capacity )
      MP_THROW_EXCEPTION( skeleton_invalid_link_handle );
# endif
    const auto entry = m_link_handles.get() + handle.index;
# ifndef MP_SKELETON_NO_HANDLE_CHECK
    if( entry->status != STATUS_ALLOCATED || entry->counter != handle.counter )
      MP_THROW_EXCEPTION( skeleton_invalid_link_handle );
# endif
    return entry->link_index;
  }

  dts_definition(typename dts_type::face_index)::get_index( face_handle handle )
  {
# ifndef MP_SKELETON_NO_HANDLE_CHECK
    if( handle.index >= m_faces_capacity )
      MP_THROW_EXCEPTION( skeleton_invalid_face_handle );
# endif
    const auto entry = m_face_handles.get() + handle.index;
# ifndef MP_SKELETON_NO_HANDLE_CHECK
    if( entry->status != STATUS_ALLOCATED || entry->counter != handle.counter )
      MP_THROW_EXCEPTION( skeleton_invalid_face_handle );
# endif
    return entry->face_index;
  }

  dts_definition(typename dts_type::atom_handle)::get_handle( atom& e )
  {
# ifndef MP_SKELETON_NO_POINTER_CHECK
    if( &e < m_atoms.get() || &e >= m_atoms.get() + m_atoms_size )
      MP_THROW_EXCEPTION( skeleton_invalid_atom_pointer );
# endif
    const auto element_index = std::distance( m_atoms.get(), &e );
# ifndef MP_SKELETON_NO_POINTER_CHECK
    if( m_atoms.get() + element_index != &e )
      MP_THROW_EXCEPTION( skeleton_invalid_atom_pointer );
# endif
    auto idx = m_atom_index_to_handle_index[ element_index ];
    return atom_handle( idx, m_atom_handles[ idx ].counter );
  }

  dts_definition(typename dts_type::link_handle)::get_handle( link& e )
  {
# ifndef MP_SKELETON_NO_POINTER_CHECK
    if( &e < m_links.get() || &e >= m_links.get() + m_links_size )
      MP_THROW_EXCEPTION( skeleton_invalid_link_pointer );
# endif
    const auto element_index = std::distance( m_links.get(), &e );
# ifndef MP_SKELETON_NO_POINTER_CHECK
    if( m_links.get() + element_index != &e )
      MP_THROW_EXCEPTION( skeleton_invalid_link_pointer );
# endif
    auto idx = m_link_index_to_handle_index[ element_index ];
    return link_handle( idx, m_link_handles[ idx ].counter );
  }

  dts_definition(typename dts_type::face_handle)::get_handle( face& e )
  {
# ifndef MP_SKELETON_NO_POINTER_CHECK
    if( &e < m_faces.get() || &e >= m_faces.get() + m_faces_size )
      MP_THROW_EXCEPTION( skeleton_invalid_face_pointer );
# endif
    const auto element_index = std::distance( m_faces.get(), &e );
# ifndef MP_SKELETON_NO_POINTER_CHECK
    if( m_faces.get() + element_index != &e )
      MP_THROW_EXCEPTION( skeleton_invalid_face_pointer );
# endif
    auto idx = m_face_index_to_handle_index[ element_index ];
    return face_handle( idx, m_face_handles[ idx ].counter );
  }

  dts_definition(typename dts_type::atom_index)::get_index( atom& e )
  {
# ifndef MP_SKELETON_NO_POINTER_CHECK
    if( &e < m_atoms.get() || &e >= m_atoms.get() + m_atoms_size )
      MP_THROW_EXCEPTION( skeleton_invalid_atom_pointer );
# endif
    const auto element_index = std::distance( m_atoms.get(), &e );
# ifndef MP_SKELETON_NO_POINTER_CHECK
    if( m_atoms.get() + element_index != &e )
      MP_THROW_EXCEPTION( skeleton_invalid_atom_pointer );
# endif
    return element_index;
  }

  dts_definition(typename dts_type::link_index)::get_index( link& e )
  {
# ifndef MP_SKELETON_NO_POINTER_CHECK
    if( &e < m_links.get() || &e >= m_links.get() + m_links_size )
      MP_THROW_EXCEPTION( skeleton_invalid_link_pointer );
# endif
    const auto element_index = std::distance( m_links.get(), &e );
# ifndef MP_SKELETON_NO_POINTER_CHECK
    if( m_links.get() + element_index != &e )
      MP_THROW_EXCEPTION( skeleton_invalid_link_pointer );
# endif
    return element_index;
  }

  dts_definition(typename dts_type::face_index)::get_index( face& e )
  {
# ifndef MP_SKELETON_NO_POINTER_CHECK
    if( &e < m_faces.get() || &e >= m_faces.get() + m_faces_size )
      MP_THROW_EXCEPTION( skeleton_invalid_face_pointer );
# endif
    const auto element_index = std::distance( m_faces.get(), &e );
# ifndef MP_SKELETON_NO_POINTER_CHECK
    if( m_faces.get() + element_index != &e )
      MP_THROW_EXCEPTION( skeleton_invalid_face_pointer );
# endif
    return element_index;
  }

  dts_definition(typename dts_type::atom&)::get_atom_by_index( atom_index index )
  {
# ifndef MP_SKELETON_NO_INDEX_CHECK
    if( index >= m_atoms_size )
      MP_THROW_EXCEPTION( skeleton_invalid_atom_index );
# endif
    return m_atoms[ index ];
  }

  dts_definition(typename dts_type::link&)::get_link_by_index( link_index index )
  {
# ifndef MP_SKELETON_NO_INDEX_CHECK
    if( index >= m_links_size )
      MP_THROW_EXCEPTION( skeleton_invalid_link_index );
# endif
    return m_links[ index ];
  }

  dts_definition(typename dts_type::face&)::get_face_by_index( face_index index )
  {
# ifndef MP_SKELETON_NO_INDEX_CHECK
    if( index >= m_faces_size )
      MP_THROW_EXCEPTION( skeleton_invalid_face_index );
# endif
    return m_faces[ index ];
  }

  dts_definition(typename dts_type::atom_handle)::get_atom_handle( atom_index index )
  {
# ifndef MP_SKELETON_NO_INDEX_CHECK
    if( index >= m_atoms_size )
      MP_THROW_EXCEPTION( skeleton_invalid_atom_index );
# endif
    auto handle_index = m_atom_index_to_handle_index[ index ];
    return atom_handle( handle_index, m_atom_handles[ handle_index ].counter );
  }

  dts_definition(typename dts_type::link_handle)::get_link_handle( link_index index )
  {
# ifndef MP_SKELETON_NO_INDEX_CHECK
    if( index >= m_links_size )
      MP_THROW_EXCEPTION( skeleton_invalid_link_index );
# endif
    auto handle_index = m_link_index_to_handle_index[ index ];
    return link_handle( handle_index, m_link_handles[ handle_index ].counter );
  }

  dts_definition(typename dts_type::face_handle)::get_face_handle( face_index index )
  {
# ifndef MP_SKELETON_NO_INDEX_CHECK
    if( index >= m_faces_size )
      MP_THROW_EXCEPTION( skeleton_invalid_face_index );
# endif
    auto handle_index = m_face_index_to_handle_index[ index ];
    return face_handle( handle_index, m_face_handles[ handle_index ].counter );
  }
