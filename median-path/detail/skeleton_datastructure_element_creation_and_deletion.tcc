namespace median_path {
  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  std::pair<
    typename skeleton_datastructure<
      atom_handle_type, atom_handle_index_bits,
      link_handle_type, link_handle_index_bits,
      face_handle_type, face_handle_index_bits>::atom_handle,
    typename skeleton_datastructure<
      atom_handle_type, atom_handle_index_bits,
      link_handle_type, link_handle_index_bits,
      face_handle_type, face_handle_index_bits>::atom& >
  skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::create_atom()
  {
    if( m_atoms_size == m_atoms_capacity )
        grow_atoms( std::min(
          max_atom_handle_index,
          m_atoms_capacity + std::max( m_atoms_capacity, atom_handle_type{10} ) ) );

    /* fetch the handle entry */
    const auto handle_index = m_atoms_next_free_handle_slot;
    auto entry = m_atom_handles + handle_index;
    m_atoms_next_free_handle_slot = entry->next_free_index;

    /* update the entry */
    ++entry->counter;
    if( entry->counter > max_atom_handle_counter )
      entry->counter = 0;
    entry->atom_index = m_atoms_size;
    entry->next_free_index = 0;
    entry->status = STATUS_ALLOCATED;
    /* map the element index to the handle entry */
    m_atom_index_to_handle_index[ m_atoms_size ] = handle_index;
    /* prepare the result */
    std::pair< atom_handle, atom& > result = {
        atom_handle( handle_index, entry->counter ),
        m_atoms[ m_atoms_size ] };
    /* update this */
    ++m_atoms_size;
    return result;
  }

  template<
        typename atom_handle_type, uint8_t atom_handle_index_bits,
        typename link_handle_type, uint8_t link_handle_index_bits,
        typename face_handle_type, uint8_t face_handle_index_bits >
    std::pair<
      typename skeleton_datastructure<
        atom_handle_type, atom_handle_index_bits,
        link_handle_type, link_handle_index_bits,
        face_handle_type, face_handle_index_bits>::link_handle,
      typename skeleton_datastructure<
        atom_handle_type, atom_handle_index_bits,
        link_handle_type, link_handle_index_bits,
        face_handle_type, face_handle_index_bits>::link& >
    skeleton_datastructure<
      atom_handle_type, atom_handle_index_bits,
      link_handle_type, link_handle_index_bits,
      face_handle_type, face_handle_index_bits>::create_link()
    {
      if( m_links_size == m_links_capacity )
        grow_links( std::min(
          max_link_handle_index,
          m_links_capacity + std::max( m_links_capacity, link_handle_type{10} ) ) );
      /* fetch the handle entry */
      const auto handle_index = m_links_next_free_handle_slot;
      auto entry = m_link_handles + handle_index;
      m_links_next_free_handle_slot = entry->next_free_index;
      /* update the entry */
      ++entry->counter;
      if( entry->counter > max_link_handle_counter )
        entry->counter = 0;
      entry->link_index = m_links_size;
      entry->next_free_index = 0;
      entry->status = STATUS_ALLOCATED;
      /* map the element index to the handle entry */
      m_link_index_to_handle_index[ m_links_size ] = handle_index;
      /* prepare the result */
      std::pair< link_handle, link& > result = {
          link_handle( handle_index, entry->counter ),
          m_links[ m_links_size ] };
      /* update this */
      ++m_links_size;
      return result;
    }

    template<
        typename atom_handle_type, uint8_t atom_handle_index_bits,
        typename link_handle_type, uint8_t link_handle_index_bits,
        typename face_handle_type, uint8_t face_handle_index_bits >
    std::pair<
      typename skeleton_datastructure<
        atom_handle_type, atom_handle_index_bits,
        link_handle_type, link_handle_index_bits,
        face_handle_type, face_handle_index_bits>::face_handle,
      typename skeleton_datastructure<
        atom_handle_type, atom_handle_index_bits,
        link_handle_type, link_handle_index_bits,
        face_handle_type, face_handle_index_bits>::face& >
    skeleton_datastructure<
      atom_handle_type, atom_handle_index_bits,
      link_handle_type, link_handle_index_bits,
      face_handle_type, face_handle_index_bits>::create_face()
    {
      if( m_faces_size == m_faces_capacity )
        grow_faces( std::min(
          max_face_handle_index,
          m_faces_capacity + std::max( m_faces_capacity, face_handle_type{10} ) ) );
      /* fetch the handle entry */
      const auto handle_index = m_faces_next_free_handle_slot;
      auto entry = m_face_handles + handle_index;
      m_faces_next_free_handle_slot = entry->next_free_index;
      /* update the entry */
      ++entry->counter;
      if( entry->counter > max_face_handle_counter )
        entry->counter = 0;
      entry->face_index = m_faces_size;
      entry->next_free_index = 0;
      entry->status = STATUS_ALLOCATED;
      /* map the element index to the handle entry */
      m_face_index_to_handle_index[ m_faces_size ] = handle_index;
      /* prepare the result */
      std::pair< face_handle, face& > result = {
          face_handle( handle_index, entry->counter ),
          m_faces[ m_faces_size ] };
      /* update this */

      ++m_faces_size;
      return result;
    }
}
