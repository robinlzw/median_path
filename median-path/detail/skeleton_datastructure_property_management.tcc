dts_definition(template<typename T>void)::add_atom_property( const std::string& name )
{
  m_atom_properties.push_back( new derived_property_buffer<T>( name ) );
  m_atom_properties.back()->resize( 0, m_atoms_capacity );
}

dts_definition(template<typename T>void)::add_link_property( const std::string& name )
{
  m_link_properties.push_back( new derived_property_buffer<T>( name ) );
  m_link_properties.back()->resize( 0, m_links_capacity );
}

dts_definition(template<typename T>void)::add_face_property( const std::string& name )
{
  m_face_properties.push_back( new derived_property_buffer<T>( name ) );
  m_face_properties.back()->resize( 0, m_faces_capacity );
}
