  dts_definition()::skeleton_datastructure() noexcept
    : m_atoms{ nullptr }, m_atom_index_to_handle_index{ nullptr }, m_atom_handles{ nullptr },
      m_links{ nullptr }, m_link_index_to_handle_index{ nullptr }, m_link_handles{ nullptr },
      m_faces{ nullptr }, m_face_index_to_handle_index{ nullptr }, m_face_handles{ nullptr },
      m_atoms_capacity{ 0 }, m_atoms_size{ 0 }, m_atoms_next_free_handle_slot{ 0 },
      m_links_capacity{ 0 }, m_links_size{ 0 }, m_links_next_free_handle_slot{ 0 },
      m_faces_capacity{ 0 }, m_faces_size{ 0 }, m_faces_next_free_handle_slot{ 0 }
  {}

  dts_definition()::skeleton_datastructure( atom_handle_type nb_atoms, link_handle_type nb_links, face_handle_type nb_faces )
	: m_atoms{ nullptr }, m_atom_index_to_handle_index{ nullptr }, m_atom_handles{ nullptr },
	  m_links{ nullptr }, m_link_index_to_handle_index{ nullptr }, m_link_handles{ nullptr },
	  m_faces{ nullptr }, m_face_index_to_handle_index{ nullptr }, m_face_handles{ nullptr },
	  m_atoms_capacity{ 0 }, m_atoms_size{ 0 }, m_atoms_next_free_handle_slot{ 0 },
	  m_links_capacity{ 0 }, m_links_size{ 0 }, m_links_next_free_handle_slot{ 0 },
	  m_faces_capacity{ 0 }, m_faces_size{ 0 }, m_faces_next_free_handle_slot{ 0 }
  {
    if( nb_atoms )
	  grow_atoms( nb_atoms );
	if( nb_links )
	  grow_links( nb_links );
	if( nb_faces )
	  grow_faces( nb_faces );
  }

  dts_definition()::~skeleton_datastructure()
  {
    for( auto& property : m_atom_properties )
      delete property;

    for( auto& property : m_link_properties )
      delete property;

    for( auto& property : m_face_properties )
      delete property;
  }
