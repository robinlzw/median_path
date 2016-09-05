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


    dts_definition(void)::remove( atom_handle h )
    {
  # ifndef MP_SKELETON_NO_HANDLE_CHECK
      if( h.index >= m_atoms_capacity )
        MP_THROW_EXCEPTION( skeleton_invalid_atom_handle );
  # endif
      auto entry = m_atom_handles + h.index;
  # ifndef MP_SKELETON_NO_HANDLE_CHECK
      if( entry->status != STATUS_ALLOCATED || entry->counter != h.counter )
        MP_THROW_EXCEPTION( skeleton_invalid_atom_handle );
  # endif

      auto index = entry->atom_index;

      // update the entry
      entry->next_free_index = m_atoms_next_free_handle_slot;
      entry->status = STATUS_FREE;
      m_atoms_next_free_handle_slot = h.index;

      // move the last atom into this one to keep the buffer tight
      if( --m_atoms_size && index != m_atoms_size )
        {
          // move the atom itself
          m_atoms[ index ] = std::move( m_atoms[ m_atoms_size ] );

          // move atom properties
          for( auto& property : m_atom_properties )
            {
              property->move( m_atoms_size, index );
            }

          // update the handle --> atom mapping
          const auto entry_index = m_atom_index_to_handle_index[ m_atoms_size ];
          entry = m_atom_handles + entry_index;
          entry->atom_index = index;
          // update the atom --> handle mapping
          m_atom_index_to_handle_index[ index ] = entry_index;
        }
      // just delete this atom
      else
        {
          m_atoms[index].~atom(); // not even necessary
          for( auto& property : m_atom_properties )
            property->destroy( index );
        }
    }

    dts_definition(void)::remove( link_handle h )
    {
# ifndef MP_SKELETON_NO_HANDLE_CHECK
    if( h.index >= m_links_capacity )
      MP_THROW_EXCEPTION( skeleton_invalid_link_handle );
# endif
    auto entry = m_link_handles + h.index;
# ifndef MP_SKELETON_NO_HANDLE_CHECK
    if( entry->status != STATUS_ALLOCATED || entry->counter != h.counter )
      MP_THROW_EXCEPTION( skeleton_invalid_link_handle );
# endif

      auto index = entry->link_index;

      // update the entry
      entry->next_free_index = m_links_next_free_handle_slot;
      entry->status = STATUS_FREE;
      m_links_next_free_handle_slot = h.index;

      // move the last link into this one to keep the buffer tight
      if( --m_links_size && index != m_links_size )
        {
          // move the link itself
          m_links[ index ] = std::move( m_links[ m_links_size ] );
          m_links[ m_links_size ] = std::move(link{});
          // move link properties
          for( auto& property : m_link_properties )
            {
              property->move( m_links_size, index );
            }

          // update the handle --> link mapping
          const auto entry_index = m_link_index_to_handle_index[ m_links_size ];
          entry = m_link_handles + entry_index;
          entry->link_index = index;
          // update the link --> handle mapping
          m_link_index_to_handle_index[ index ] = entry_index;
        }
      // just delete this link
      else
        {
          m_links[index] = std::move(link{});
          for( auto& property : m_link_properties )
            property->destroy( index );
        }
    }

    dts_definition(void)::remove( face_handle h )
    {
# ifndef MP_SKELETON_NO_HANDLE_CHECK
    if( h.index >= m_faces_capacity )
      MP_THROW_EXCEPTION( skeleton_invalid_face_handle );
# endif
    auto entry = m_face_handles + h.index;
# ifndef MP_SKELETON_NO_HANDLE_CHECK
    if( entry->status != STATUS_ALLOCATED || entry->counter != h.counter )
      MP_THROW_EXCEPTION( skeleton_invalid_face_handle );
# endif

      auto index = entry->face_index;

      // update the entry
      entry->next_free_index = m_faces_next_free_handle_slot;
      entry->status = STATUS_FREE;
      m_faces_next_free_handle_slot = h.index;

      // move the last face into this one to keep the buffer tight
      if( --m_faces_size && index != m_faces_size )
        {
          // move the face itself
          m_faces[ index ] = std::move( m_faces[ m_faces_size ] );
          m_faces[ m_faces_size ] = std::move(face{});
          // move face properties
          for( auto& property : m_face_properties )
            {
              property->move( m_faces_size, index );
            }

          // update the handle --> face mapping
          const auto entry_index = m_face_index_to_handle_index[ m_faces_size ];
          entry = m_face_handles + entry_index;
          entry->face_index = index;
          // update the face --> handle mapping
          m_face_index_to_handle_index[ index ] = entry_index;
        }
      // just delete this face
      else
        {
          m_faces[ index ] = std::move(face{});
          for( auto& property : m_face_properties )
            property->destroy( index );
        }
    }

    dts_definition(void)::remove_atom_by_index( atom_handle_type index )
    {
  # ifndef MP_SKELETON_NO_INDEX_CHECK
        if( index >= m_atoms_size )
          MP_THROW_EXCEPTION( skeleton_invalid_atom_index );
  # endif
        // update the entry
        {
          const auto entry_index = m_atom_index_to_handle_index[ index ];
          auto entry = m_atom_handles + entry_index;
          entry->next_free_index = m_atoms_next_free_handle_slot;
          entry->status = STATUS_FREE;
          m_atoms_next_free_handle_slot = entry_index;
        }

        // move the last atom into this one to keep the buffer tight
        if( --m_atoms_size && index != m_atoms_size )
          {
            // move the atom itself
            m_atoms[ index ] = std::move( m_atoms[ m_atoms_size ] );

            // move atom properties
            for( auto& property : m_atom_properties )
              {
                property->move( m_atoms_size, index );
              }

            // update the handle --> atom mapping
            const auto entry_index = m_atom_index_to_handle_index[ m_atoms_size ];
            auto entry = m_atom_handles + entry_index;
            entry->atom_index = index;
            // update the atom --> handle mapping
            m_atom_index_to_handle_index[ index ] = entry_index;
          }
        // just delete this atom
        else
          {
            for( auto& property : m_atom_properties )
              property->destroy( index );
          }
    }

    dts_definition(void)::remove_link_by_index( link_handle_type index )
    {
  # ifndef MP_SKELETON_NO_INDEX_CHECK
        if( index >= m_links_size )
          MP_THROW_EXCEPTION( skeleton_invalid_link_index );
  # endif
        // update the entry
        {
          const auto entry_index = m_link_index_to_handle_index[ index ];
          auto entry = m_link_handles + entry_index;
          entry->next_free_index = m_links_next_free_handle_slot;
          entry->status = STATUS_FREE;
          m_links_next_free_handle_slot = entry_index;
        }

        // move the last link into this one to keep the buffer tight
        if( --m_links_size && index != m_links_size )
          {
            // move the link itself
            m_links[ index ] = std::move( m_links[ m_links_size ] );
            m_links[ m_links_size ] = std::move(link{});

            // move link properties
            for( auto& property : m_link_properties )
              {
                property->move( m_links_size, index );
              }

            // update the handle --> link mapping
            const auto entry_index = m_link_index_to_handle_index[ m_links_size ];
            auto entry = m_link_handles + entry_index;
            entry->link_index = index;
            // update the link --> handle mapping
            m_link_index_to_handle_index[ index ] = entry_index;
          }
        // just delete this link
        else
          {
            m_links[ index ] = std::move(link{});
            for( auto& property : m_link_properties )
              property->destroy( index );
          }
    }

    dts_definition(void)::remove_face_by_index( face_handle_type index )
    {
  # ifndef MP_SKELETON_NO_INDEX_CHECK
        if( index >= m_faces_size )
          MP_THROW_EXCEPTION( skeleton_invalid_face_index );
  # endif
        // update the entry
        {
          const auto entry_index = m_face_index_to_handle_index[ index ];
          auto entry = m_face_handles + entry_index;
          entry->next_free_index = m_faces_next_free_handle_slot;
          entry->status = STATUS_FREE;
          m_faces_next_free_handle_slot = entry_index;
        }

        // move the last face into this one to keep the buffer tight
        if( --m_faces_size && index != m_faces_size )
          {
            // move the face itself
            m_faces[ index ] = std::move( m_faces[ m_faces_size ] );
            m_faces[ m_faces_size ] = std::move(face{});
            // move face properties
            for( auto& property : m_face_properties )
              {
                property->move( m_faces_size, index );
              }

            // update the handle --> face mapping
            const auto entry_index = m_face_index_to_handle_index[ m_faces_size ];
            auto entry = m_face_handles + entry_index;
            entry->face_index = index;
            // update the face --> handle mapping
            m_face_index_to_handle_index[ index ] = entry_index;
          }
        // just delete this face
        else
          {
            m_faces[ index ] = std::move(face{});
            for( auto& property : m_face_properties )
              property->destroy( index );
          }
    }

    dts_definition(void)::remove( atom& e )
    {
  # ifndef MP_SKELETON_NO_POINTER_CHECK
      if( &e < m_atoms || &e >= m_atoms + m_atoms_size )
        MP_THROW_EXCEPTION( skeleton_invalid_atom_pointer );
  # endif
      const atom_handle_type index = std::distance( m_atoms, &e );
  # ifndef MP_SKELETON_NO_POINTER_CHECK
      if( m_atoms + index != &e )
        MP_THROW_EXCEPTION( skeleton_invalid_atom_pointer );
  # endif

      // update the entry
      {
        const auto entry_index = m_atom_index_to_handle_index[ index ];
        auto entry = m_atom_handles + entry_index;
        entry->next_free_index = m_atoms_next_free_handle_slot;
        entry->status = STATUS_FREE;
        m_atoms_next_free_handle_slot = entry_index;
      }

      // move the last atom into this one to keep the buffer tight
      if( --m_atoms_size && index != m_atoms_size )
        {
          // move the atom itself
          e = std::move( m_atoms[ m_atoms_size ] );

          // move atom properties
          for( auto& property : m_atom_properties )
            {
              property->move( m_atoms_size, index );
            }

          // update the handle --> atom mapping
          const auto entry_index = m_atom_index_to_handle_index[ m_atoms_size ];
          auto entry = m_atom_handles + entry_index;
          entry->atom_index = index;
          // update the atom --> handle mapping
          m_atom_index_to_handle_index[ index ] = entry_index;
        }
      // just delete this atom
      else
        {
          for( auto& property : m_atom_properties )
            property->destroy( index );
        }
    }

    dts_definition(void)::remove( link& e )
    {
  # ifndef MP_SKELETON_NO_POINTER_CHECK
      if( &e < m_links || &e >= m_links + m_links_size )
        MP_THROW_EXCEPTION( skeleton_invalid_link_pointer );
  # endif
      const link_handle_type index = std::distance( m_links, &e );
  # ifndef MP_SKELETON_NO_POINTER_CHECK
      if( m_links + index != &e )
        MP_THROW_EXCEPTION( skeleton_invalid_link_pointer );
  # endif

      // update the entry
      {
        const auto entry_index = m_link_index_to_handle_index[ index ];
        auto entry = m_link_handles + entry_index;
        entry->next_free_index = m_links_next_free_handle_slot;
        entry->status = STATUS_FREE;
        m_links_next_free_handle_slot = entry_index;
      }

      // move the last link into this one to keep the buffer tight
      if( --m_links_size && index != m_links_size )
        {
          // move the link itself
          e = std::move( m_links[ m_links_size ] );
          m_links[ m_links_size ] = std::move(link{});
          // move link properties
          for( auto& property : m_link_properties )
            {
              property->move( m_links_size, index );
            }

          // update the handle --> link mapping
          const auto entry_index = m_link_index_to_handle_index[ m_links_size ];
          auto entry = m_link_handles + entry_index;
          entry->link_index = index;
          // update the link --> handle mapping
          m_link_index_to_handle_index[ index ] = entry_index;
        }
      // just delete this link
      else
        {
          m_links[ index ] = std::move(link{});
          for( auto& property : m_link_properties )
            property->destroy( index );
        }
    }

    dts_definition(void)::remove( face& e )
    {
  # ifndef MP_SKELETON_NO_POINTER_CHECK
      if( &e < m_faces || &e >= m_faces + m_faces_size )
        MP_THROW_EXCEPTION( skeleton_invalid_face_pointer );
  # endif
      const face_handle_type index = std::distance( m_faces, &e );
  # ifndef MP_SKELETON_NO_POINTER_CHECK
      if( m_faces + index != &e )
        MP_THROW_EXCEPTION( skeleton_invalid_face_pointer );
  # endif

      // update the entry
      {
        const auto entry_index = m_face_index_to_handle_index[ index ];
        auto entry = m_face_handles + entry_index;
        entry->next_free_index = m_faces_next_free_handle_slot;
        entry->status = STATUS_FREE;
        m_faces_next_free_handle_slot = entry_index;
      }

      // move the last face into this one to keep the buffer tight
      if( --m_faces_size && index != m_faces_size )
        {
          // move the face itself
          e = std::move( m_faces[ m_faces_size ] );
          m_faces[ m_faces_size ] = std::move(face{});

          // move face properties
          for( auto& property : m_face_properties )
            {
              property->move( m_faces_size, index );
            }

          // update the handle --> face mapping
          const auto entry_index = m_face_index_to_handle_index[ m_faces_size ];
          auto entry = m_face_handles + entry_index;
          entry->face_index = index;
          // update the face --> handle mapping
          m_face_index_to_handle_index[ index ] = entry_index;
        }
      // just delete this face
      else
        {
          m_faces[ index ] = std::move(face{});
          for( auto& property : m_face_properties )
            property->destroy( index );
        }
    }
