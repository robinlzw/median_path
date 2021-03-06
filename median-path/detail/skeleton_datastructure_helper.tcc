 dts_definition(void)::clear( atom_handle_type atom_capacity,
                              link_handle_type link_capacity,
                              face_handle_type face_capacity )
 {
   clear_atoms( atom_capacity );
   clear_links( link_capacity );
   clear_faces( face_capacity );
 }

 dts_definition(void)::clear_atoms( atom_handle_type atom_capacity )
 {
   if( atom_capacity <= m_atoms_capacity )
     {
       # pragma omp parallel for
       for( size_t i = 0; i < m_atoms_capacity; ++ i )
         {
           auto& entry = m_atom_handles[ i ];
           entry.status = STATUS_FREE;
           entry.next_free_index = i + 1;

         }

       for( auto& property : m_atom_properties )
         property->clear( m_atoms_capacity );

       m_atoms_size = 0;
       m_atoms_next_free_handle_slot = 0;
     }
   else
     {
       m_atoms.reset( nullptr );
       m_atom_handles.reset( nullptr );
       m_atom_index_to_handle_index.reset( nullptr );

       m_atoms_capacity = 0;
       m_atoms_size = 0;
       m_atoms_next_free_handle_slot = 0;

       grow_atoms( atom_capacity );
     }
 }

 dts_definition(void)::clear_links( link_handle_type link_capacity )
  {
    if( link_capacity <= m_links_capacity )
      {
        # pragma omp parallel for
        for( size_t i = 0; i < m_links_capacity; ++ i )
          {
            auto& entry = m_link_handles[ i ];
            entry.status = STATUS_FREE;
            entry.next_free_index = i + 1;

          }

        for( auto& property : m_link_properties )
          property->clear( m_links_capacity );

        m_links_size = 0;
        m_links_next_free_handle_slot = 0;
      }
    else
      {
        m_links.reset( nullptr );
        m_link_handles.reset( nullptr );
        m_link_index_to_handle_index.reset( nullptr );

        m_links_capacity = 0;
        m_links_size = 0;
        m_links_next_free_handle_slot = 0;

        grow_links( link_capacity );
      }
  }

 dts_definition(void)::clear_faces( face_handle_type face_capacity )
  {
    if( face_capacity <= m_faces_capacity )
      {
        # pragma omp parallel for
        for( size_t i = 0; i < m_faces_capacity; ++ i )
          {
            auto& entry = m_face_handles[ i ];
            entry.status = STATUS_FREE;
            entry.next_free_index = i + 1;

          }

        for( auto& property : m_face_properties )
          property->clear( m_faces_capacity );

        m_faces_size = 0;
        m_faces_next_free_handle_slot = 0;
      }
    else
      {
        m_faces = nullptr;
        m_face_handles = nullptr;
        m_face_index_to_handle_index = nullptr;

        m_faces_capacity = 0;
        m_faces_size = 0;
        m_faces_next_free_handle_slot = 0;

        grow_faces( face_capacity );
      }
  }

  dts_definition(void)::grow_atoms( atom_handle_type new_capacity )
  {
    // check for overflow
    if( new_capacity <= m_atoms_capacity || new_capacity > max_atom_handle_index )
      MP_THROW_EXCEPTION( skeleton_atom_buffer_overflow );

    try {
      // initialize data
      if( !m_atoms )
        {
          m_atoms = std::make_unique<atom[]>( new_capacity );
          m_atom_index_to_handle_index = std::make_unique<atom_handle_type[]>( new_capacity );
          m_atom_handles = std::make_unique<atom_handle_entry[]>( new_capacity );
        }
      // resize buffers
      else
        {
          auto new_element_buffer = std::make_unique<atom[]>( new_capacity );
          auto new_element_to_handle = std::make_unique<atom_handle_type[]>( new_capacity );
          auto new_handle_buffer = std::make_unique<atom_handle_entry[]>( new_capacity );
          # pragma omp parallel for
          for( atom_handle_type i = 0; i < m_atoms_capacity; ++ i )
            {
              new_element_buffer[ i ] = std::move( m_atoms[ i ] );
              new_element_to_handle[ i ] = m_atom_index_to_handle_index[ i ];
              new_handle_buffer[ i ] = m_atom_handles[ i ];
            }
          m_atoms = std::move(new_element_buffer);
          m_atom_index_to_handle_index = std::move(new_element_to_handle);
          m_atom_handles = std::move(new_handle_buffer);
        }
    } catch( std::bad_alloc& ba ) {
      MP_THROW_EXCEPTION( skeleton_atom_buffer_overflow );
    }

    # pragma omp parallel for
    for( size_t i = m_atoms_capacity; i < new_capacity; ++ i )
      m_atom_handles[ i ].next_free_index = i + 1;

    const auto nb_properties = m_atom_properties.size();
    # pragma omp parallel for
    for( size_t i = 0; i < nb_properties; ++ i )
      {
        m_atom_properties[i]->resize( m_atoms_capacity, new_capacity );
      }
    m_atoms_capacity = new_capacity;
  }


  dts_definition(void)::grow_links( link_handle_type new_capacity )
  {
    // check for overflow
    if( new_capacity <= m_links_capacity || new_capacity > max_link_handle_index )
      MP_THROW_EXCEPTION( skeleton_link_buffer_overflow );

    try {
      // initialize data
      if( !m_links )
        {
          m_links = std::make_unique<link[]>( new_capacity );
          m_link_index_to_handle_index = std::make_unique<link_handle_type[]>( new_capacity );
          m_link_handles = std::make_unique<link_handle_entry[]>( new_capacity );
        }
      // resize buffers
      else
        {
          auto new_element_buffer = std::make_unique<link[]>( new_capacity );
          auto new_element_to_handle = std::make_unique<link_handle_type[]>( new_capacity );
          auto new_handle_buffer = std::make_unique<link_handle_entry[]>( new_capacity );
          # pragma omp parallel for
          for( link_handle_type i = 0; i < m_links_capacity; ++ i )
            {
              new_element_buffer[ i ] = std::move( m_links[ i ] );
              new_element_to_handle[ i ] = m_link_index_to_handle_index[ i ];
              new_handle_buffer[ i ] = m_link_handles[ i ];
            }
          m_links = std::move(new_element_buffer);
          m_link_index_to_handle_index = std::move(new_element_to_handle);
          m_link_handles = std::move(new_handle_buffer);
        }
    } catch( std::bad_alloc& ba ) {
      MP_THROW_EXCEPTION( skeleton_link_buffer_overflow );
    }

    # pragma omp parallel for
    for( size_t i = m_links_capacity; i < new_capacity; ++ i )
      m_link_handles[ i ].next_free_index = i + 1;

    const auto nb_properties = m_link_properties.size();
    # pragma omp parallel for
    for( size_t i = 0; i < nb_properties; ++ i )
      {
        m_link_properties[i]->resize( m_links_capacity, new_capacity );
      }
    m_links_capacity = new_capacity;
  }

  dts_definition(void)::grow_faces( face_handle_type new_capacity )
  {
    // check for overflow
    if( new_capacity <= m_faces_capacity || new_capacity > max_face_handle_index )
      MP_THROW_EXCEPTION( skeleton_face_buffer_overflow );

    try {
      // initialize data
      if( !m_faces )
        {
          m_faces = std::make_unique<face[]>( new_capacity );
          m_face_index_to_handle_index = std::make_unique<face_handle_type[]>( new_capacity );
          m_face_handles = std::make_unique<face_handle_entry[]>( new_capacity );
        }
      // resize buffers
      else
        {
          auto new_element_buffer = std::make_unique<face[]>( new_capacity );
          auto new_element_to_handle = std::make_unique<face_handle_type[]>( new_capacity );
          auto new_handle_buffer = std::make_unique<face_handle_entry[]>( new_capacity );
          # pragma omp parallel for
          for( face_handle_type i = 0; i < m_faces_capacity; ++ i )
            {
              new_element_buffer[ i ] = std::move( m_faces[ i ] );
              new_element_to_handle[ i ] = m_face_index_to_handle_index[ i ];
              new_handle_buffer[ i ] = m_face_handles[ i ];
            }
          m_faces = std::move(new_element_buffer);
          m_face_index_to_handle_index = std::move(new_element_to_handle);
          m_face_handles = std::move(new_handle_buffer);
        }
    } catch( std::bad_alloc& ba ) {
      MP_THROW_EXCEPTION( skeleton_face_buffer_overflow );
    }

    # pragma omp parallel for
    for( size_t i = m_faces_capacity; i < new_capacity; ++ i )
      m_face_handles[ i ].next_free_index = i + 1;

    const auto nb_properties = m_face_properties.size();
    # pragma omp parallel for
    for( size_t i = 0; i < nb_properties; ++ i )
      {
        m_face_properties[i]->resize( m_faces_capacity, new_capacity );
      }
    m_faces_capacity = new_capacity;
  }
