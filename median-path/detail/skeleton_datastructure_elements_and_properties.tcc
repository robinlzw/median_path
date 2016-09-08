  dts_definition()::link::link()
    : h1{}, h2{}
  {}

  dts_definition(typename dts_type::link&)::link::operator =(link&& other )
  {
    h1 = other.h1;
    h2 = other.h2;
    return *this;
  }

  dts_definition()::face::face()
    : atoms{ atom_handle{}, atom_handle{}, atom_handle{} },
      links{ link_handle{}, link_handle{}, link_handle{} }
  {}

  dts_definition(typename dts_type::face&)::face::operator =(face&& other )
  {
    atoms[0] = other.atoms[0];
    atoms[1] = other.atoms[1];
    atoms[2] = other.atoms[2];

    links[0] = other.links[0];
    links[1] = other.links[1];
    links[2] = other.links[2];

    return *this;
  }

  template< typename T >
  inline T& base_property_buffer::get(size_t index )
  {
    return reinterpret_cast< T* >( m_buffer )[ index ];
  }

  template< typename T >
  derived_property_buffer<T>::derived_property_buffer( const std::string& name )
    : base_property_buffer( sizeof( value_type ), name )
  {}


  template< typename T >
  void derived_property_buffer<T>::destroy( size_t index )
  {
    reinterpret_cast< pointer_type >( base_property_buffer::m_buffer )[ index ] = std::move( value_type() );
  }

  template< typename T >
  void derived_property_buffer<T>::destroy( size_t from, size_t end )
  {
    auto start = reinterpret_cast< pointer_type >( base_property_buffer::m_buffer ) + from,
          last = reinterpret_cast< pointer_type >( base_property_buffer::m_buffer ) + end;
    while( start < last )
      {
        *start = std::move( value_type() );
        ++start;
      }
  }

  template< typename T >
  void derived_property_buffer<T>::resize( size_t old_capacity, size_t new_capacity )
  {
    if( old_capacity )
      {
        auto new_buffer = new value_type[ new_capacity ];
        for( size_t i = 0; i < old_capacity; ++ i )
          {
            new_buffer[ i ] = std::move( reinterpret_cast< pointer_type >( base_property_buffer::m_buffer )[ i ] );
          }
        delete[] reinterpret_cast< pointer_type >( base_property_buffer::m_buffer );
        base_property_buffer::m_buffer = reinterpret_cast< unsigned char* >( new_buffer );
      }
    else
      {
        base_property_buffer::m_buffer = reinterpret_cast< unsigned char* >( new value_type[ new_capacity ] );
      }
  }

  template< typename T >
  void derived_property_buffer<T>::move( size_t from, size_t to )
  {
    // copy to destination thanks to a move operator
    reinterpret_cast< pointer_type >( base_property_buffer::m_buffer )[ to ] =
        std::move( reinterpret_cast< pointer_type >( base_property_buffer::m_buffer )[ from ] );
    // put the source into a default state thanks to the default constructor
    reinterpret_cast< pointer_type >( base_property_buffer::m_buffer )[ from ] = std::move( value_type() );
  }

  template<typename T>
  derived_property_buffer<T>::~derived_property_buffer()
  {
    delete[] reinterpret_cast<T*>( base_property_buffer::m_buffer );
  }
