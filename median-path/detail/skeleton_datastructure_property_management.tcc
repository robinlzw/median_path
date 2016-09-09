dts_definition(template<typename T> base_property_buffer*)::add_atom_property( const std::string& name )
{
  auto end = m_atom_properties.end();
  for( auto it = m_atom_properties.begin(); it != end; ++ it )
    if( (*it)->m_name == name )
      MP_THROW_EXCEPTION( skeleton_atom_property_already_exist );
  m_atom_properties.push_back( std::make_unique<derived_property_buffer<T>>( name ) );
  m_atom_properties.back()->resize( 0, m_atoms_capacity );
  return m_atom_properties.back().get();
}

dts_definition(template<typename T>base_property_buffer*)::add_link_property( const std::string& name )
{
  auto end = m_link_properties.end();
  for( auto it = m_link_properties.begin(); it != end; ++ it )
    if( (*it)->m_name == name )
      MP_THROW_EXCEPTION( skeleton_link_property_already_exist );
  m_link_properties.push_back( std::make_unique<derived_property_buffer<T>>( name ) );
  m_link_properties.back()->resize( 0, m_links_capacity );
  return m_link_properties.back().get();
}

dts_definition(template<typename T>base_property_buffer*)::add_face_property( const std::string& name )
{
  auto end = m_face_properties.end();
  for( auto it = m_face_properties.begin(); it != end; ++ it )
    if( (*it)->m_name == name )
      MP_THROW_EXCEPTION( skeleton_face_property_already_exist );
  m_face_properties.push_back( std::make_unique<derived_property_buffer<T>>( name ) );
  m_face_properties.back()->resize( 0, m_faces_capacity );
  return m_face_properties.back().get();
}

dts_definition(void)::remove_atom_property( const std::string& name )
{
  auto end = m_atom_properties.end();
  for( auto it = m_atom_properties.begin(); it != end; ++ it)
    {
      if( (*it)->m_name == name )
        {
          m_atom_properties.erase( it );
          return;
        }
    }
  MP_THROW_EXCEPTION( skeleton_invalid_atom_property_name );
}
dts_definition(void)::remove_link_property( const std::string& name )
{
  auto end = m_link_properties.end();
  for( auto it = m_link_properties.begin(); it != end; ++ it)
    {
      if( (*it)->m_name == name )
        {
          m_link_properties.erase( it );
          return;
        }
    }
  MP_THROW_EXCEPTION( skeleton_invalid_link_property_name );
}
dts_definition(void)::remove_face_property( const std::string& name )
{
  auto end = m_face_properties.end();
  for( auto it = m_face_properties.begin(); it != end; ++ it)
    {
      if( (*it)->m_name == name )
        {
          m_face_properties.erase( it );
          return;
        }
    }
  MP_THROW_EXCEPTION( skeleton_invalid_face_property_name );
}

dts_definition(void)::remove_atom_property( size_t property_index )
{
  if( property_index >= m_atom_properties.size() )
    MP_THROW_EXCEPTION( skeleton_invalid_atom_property_index );
  m_atom_properties.erase( m_atom_properties.begin() + property_index );
}
dts_definition(void)::remove_link_property( size_t property_index )
{
  if( property_index >= m_link_properties.size() )
    MP_THROW_EXCEPTION( skeleton_invalid_link_property_index );
  m_link_properties.erase( m_link_properties.begin() + property_index );
}
dts_definition(void)::remove_face_property( size_t property_index )
{
  if( property_index >= m_face_properties.size() )
    MP_THROW_EXCEPTION( skeleton_invalid_face_property_index );
  m_face_properties.erase( m_face_properties.begin() + property_index );
}

dts_definition(void)::remove_atom_property( base_property_buffer& property )
{
  auto end = m_atom_properties.end();
  for( auto it = m_atom_properties.begin(); it != end; ++ it)
    {
      if( (*it).get() == &property )
        {
          m_atom_properties.erase( it );
          return;
        }
    }
  MP_THROW_EXCEPTION( skeleton_invalid_atom_property_pointer );
}
dts_definition(void)::remove_link_property( base_property_buffer& property )
{
  auto end = m_link_properties.end();
  for( auto it = m_link_properties.begin(); it != end; ++ it)
    {
      if( (*it).get() == &property )
        {
          m_link_properties.erase( it );
          return;
        }
    }
  MP_THROW_EXCEPTION( skeleton_invalid_link_property_pointer );
}
dts_definition(void)::remove_face_property( base_property_buffer& property )
{
  auto end = m_face_properties.end();
  for( auto it = m_face_properties.begin(); it != end; ++ it)
    {
      if( (*it).get() == &property )
        {
          m_face_properties.erase( it );
          return;
        }
    }
  MP_THROW_EXCEPTION( skeleton_invalid_face_property_pointer );
}
