/* Created on: Mar 1, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# ifndef MEDIAN_PATH_SKELETON_DATASTRUCTURE_H_
# define MEDIAN_PATH_SKELETON_DATASTRUCTURE_H_

# include "../median_path.h"
# include "exceptions.h"

# include <graphics-origin/geometry/vec.h>

# include <type_traits>
# include <vector>

BEGIN_MP_NAMESPACE

  /**
   * Design thoughts:
   * Designing a efficient and usable skeleton data structure is not so easy. I
   * tried many times and failed :-). Here are the thoughts that led me to
   * this design.
   *
   * We have three categories of data:
   * 1. atom related
   * 2. link related
   * 3. face related.
   *
   * For each category, the data is accessible thanks to handle and is tightly
   * in a buffer. Data can be added to a category dynamically, but there are
   * some data that will always be stored in a skeleton:
   *
   * - atoms
   * An atom is basically a vec4 augmented with some methods
   * This is the data that will be the more often used and transferred (e.g.:
   * to the gpu for rendering or computing)
   *
   * - links
   * A link is a connection between two atoms. How to represent that?
   * Well, we have to keep in mind that links could be added and removed.
   * Also, we do not want to have an adjacency matrix for that since a)
   * the number of atoms could be pretty high and b) atoms will be added
   * and removed too. Thus, we should store something similar to an adjacency
   * list. Due to the link removal features, it is not practical to store
   * all links of the same atom contiguously in only one buffer. Each atom
   * will store its neighbors in a buffer of vectors.
   *
   * Still, each link should be identified by an handle to be able to index
   * link data. It is tempting to use the index part of the two atom handles
   * to build a link handle. However, in this case we could not use the
   * index part of the link handle to access to a buffer. We need then an
   * handle entry buffer (that maps the index of an handle to the index of
   * a link data in a tight buffer). Also, we need to be able to get the
   * two atom handles of a link thanks to its link handle: this will be the
   * link structure. The skeleton class will store this information as an
   * atom property.
   *
   * - faces
   * I consider only triangular faces. Some structuration algorithms produce
   * faces that are not triangular, but there is currently no need for non
   * triangular faces. Thus a face connects three vertices and three links.
   * So an atom must know about its faces, a link must also know about its
   * faces: those information are respectively atom data and link data and
   * stored by the skeleton class.
   */
  template<
    typename atom_handle_type, uint8_t atom_handle_index_bits,
    typename link_handle_type, uint8_t link_handle_index_bits,
    typename face_handle_type, uint8_t face_handle_index_bits >
  struct skeleton_datastructure {

    /**************************************************************************
     * CHECK TEMPLATE PARAMETERS:                                             *
     *  - handle storage types must be integral and unsigned, they should     *
     *  also be big enough to have the requested number of bits for indices.  *
     **************************************************************************/
    static_assert(
        std::is_integral< atom_handle_type >::value
     && std::is_unsigned< atom_handle_type >::value,
     "atom handle type must be an unsigned integral");

    static_assert(
        std::is_integral< link_handle_type >::value
     && std::is_unsigned< link_handle_type >::value,
     "link handle type must be an unsigned integral");

    static_assert(
        std::is_integral< face_handle_type >::value
     && std::is_unsigned< face_handle_type >::value,
     "face handle type must be an unsigned integral");

    static_assert(
      atom_handle_index_bits + 2 < sizeof( atom_handle_type ) * 8,
      "atom handle type is not large enough to have the requested number of bits for the index"
    );

    static_assert(
      link_handle_index_bits + 2 < sizeof( link_handle_type ) * 8,
      "link handle type is not large enough to have the requested number of bits for the index"
    );

    static_assert(
      face_handle_index_bits + 2 < sizeof( face_handle_type ) * 8,
      "face handle type is not large enough to have the requested number of bits for the index"
    );

    static constexpr uint8_t atom_handle_bits = sizeof( atom_handle_type ) << 3;
    static constexpr atom_handle_type max_atom_handle_index = ( 1U << atom_handle_index_bits ) - 1;
    static constexpr atom_handle_type max_atom_handle_counter = ( 1U << (atom_handle_bits - atom_handle_index_bits - 2 )) - 1;

    static constexpr uint8_t link_handle_bits = sizeof( link_handle_type ) << 3;
    static constexpr link_handle_type max_link_handle_index = ( 1U << link_handle_index_bits ) - 1;
    static constexpr link_handle_type max_link_handle_counter = ( 1U << (link_handle_bits - link_handle_index_bits - 2 )) - 1;

    static constexpr uint8_t face_handle_bits = sizeof( face_handle_type ) << 3;
    static constexpr face_handle_type max_face_handle_index = ( 1U << face_handle_index_bits ) - 1;
    static constexpr face_handle_type max_face_handle_counter = ( 1U << (face_handle_bits - face_handle_index_bits - 2 )) - 1;

    enum{ STATUS_FREE = 0, STATUS_ALLOCATED = 1, STATUS_GARBAGE = 2 };

    /**************************************************************************
     * HANDLE ENTRIES:                                                        *
     *  - connect an handle to an element in the tight buffer                 *
     *  - store information to allocated handles                              *
     *  - store the status of the element in the tight buffer                 *
     **************************************************************************/
    struct atom_handle_entry {
      atom_handle_type next_free_index : atom_handle_index_bits;
      atom_handle_type counter         : atom_handle_bits - atom_handle_index_bits - 2;
      atom_handle_type status          : 2;

      atom_handle_type atom_index;

      atom_handle_entry()
        : next_free_index{ 0 }, counter{ 0 },
          status{ STATUS_FREE }, atom_index{ 0 }
      {}
    };
    struct link_handle_entry {
      link_handle_type next_free_index : link_handle_index_bits;
      link_handle_type counter         : link_handle_bits - link_handle_index_bits - 2;
      link_handle_type status          : 2;

      link_handle_type link_index;

      link_handle_entry()
        : next_free_index{ 0 }, counter{ 0 },
          status{ STATUS_FREE }, link_index{ 0 }
      {}
    };
    struct face_handle_entry {
      face_handle_type next_free_index : face_handle_index_bits;
      face_handle_type counter         : face_handle_bits - face_handle_index_bits - 2;
      face_handle_type status          : 2;

      face_handle_type face_index;

      face_handle_entry()
        : next_free_index{ 0 }, counter{ 0 },
          status{ STATUS_FREE }, face_index{ 0 }
      {}
    };

    /**************************************************************************
     * HANDLES:                                                               *
     *  - identifiers of an element                                           *
     **************************************************************************/
    struct atom_handle {
      atom_handle_type index  : atom_handle_index_bits;
      atom_handle_type counter: atom_handle_bits - atom_handle_index_bits;
      atom_handle()
        : index{ max_atom_handle_index },
          counter{ max_atom_handle_counter + 1 }
      {}
      atom_handle( atom_handle_type idx, atom_handle_type ctr )
        : index{ idx }, counter{ ctr }
      {}
      inline operator atom_handle_type() const
      {
        return (atom_handle_type(counter) << atom_handle_index_bits ) | index;
      }
      bool is_valid() const noexcept
      {
        return counter <= max_atom_handle_counter;
      }
    };
    struct link_handle {
      link_handle_type index  : link_handle_index_bits;
      link_handle_type counter: link_handle_bits - link_handle_index_bits;
      link_handle()
        : index{ max_link_handle_index},
          counter{ max_link_handle_counter + 1 }
      {}
      link_handle( link_handle_type idx, link_handle_type ctr )
        : index{ idx }, counter{ ctr }
      {}
      inline operator link_handle_type() const
      {
        return (link_handle_type(counter) << link_handle_index_bits ) | index;
      }
      bool is_valid() const noexcept
      {
        return counter <= max_link_handle_counter;
      }
    };
    struct face_handle {
      face_handle_type index  : face_handle_index_bits;
      face_handle_type counter: face_handle_bits - face_handle_index_bits;
      face_handle()
        : index{ max_face_handle_index },
          counter{ max_face_handle_counter + 1}
      {}
      face_handle( face_handle_type idx, face_handle_type ctr )
        : index{ idx }, counter{ ctr }
      {}
      inline operator face_handle_type() const
      {
        return (face_handle_type(counter) << face_handle_index_bits ) | index;
      }
      bool is_valid() const noexcept
      {
        return counter <= max_face_handle_counter;
      }
    };

    /**************************************************************************
     * ELEMENT TYPES:                                                         *
     *  - an atom is a ball                                                   *
     *  - a link is a pair of atom handles                                    *
     *  - a face is a 3-cycle of links along with their atom handles          *
     **************************************************************************/
    struct atom
      : public GO_NAMESPACE::vec4 {
      atom& operator=( atom&& other )
      {
        GO_NAMESPACE::vec4::operator=( std::move(other) );
        return *this;
      }

      atom()
        : GO_NAMESPACE::vec4{}
      {}

      atom( const GO_NAMESPACE::vec3& center, const real& radius )
        : GO_NAMESPACE::vec4{ center, radius }
      {}

      atom( const GO_NAMESPACE::vec4& ball )
        : GO_NAMESPACE::vec4{ ball }
      {}
    };
    struct link {

      link()
        : h1{}, h2{}
      {}

      link& operator=( link&& other )
      {
        h1 = other.h1;
        h2 = other.h2;
        return *this;
      }

      atom_handle h1;
      atom_handle h2;
    };
    struct face {

      face()
        : atoms{ atom_handle{}, atom_handle{}, atom_handle{} },
          links{ link_handle{}, link_handle{}, link_handle{} }
      {}

      face& operator=( face&& other )
      {
        atoms[0] = other.atoms[0];
        atoms[1] = other.atoms[1];
        atoms[2] = other.atoms[2];

        links[0] = other.links[0];
        links[1] = other.links[1];
        links[2] = other.links[2];

        return *this;
      }

      atom_handle atoms[ 3 ];
      link_handle links[ 3 ];
    };

    /**************************************************************************
     * ELEMENT DATA TYPES:                                                    *
     *  - a common structure to be stored in a container                      *
     *  - six virtual functions to be able to call the constructor and        *
     *  destructors of the actual types:                                      *
     *     a) the property buffer destructor                                  *
     *     b) the property buffer resize                                      *
     *     c) the property buffer clear                                       *
     *     d) move an element inside the property buffer                      *
     *     e) destroy a particular element in the buffer                      *
     *     f) destroy a range of particular elements in the buffer            *
     **************************************************************************/
    struct base_property_buffer {
      base_property_buffer( size_t size, const std::string& name )
        : m_sizeof_element{ size }, m_name{ name },
          m_buffer{ nullptr }
      {}

      virtual ~base_property_buffer() {}

      template< typename T >
      T& get( size_t index )
      {
        return reinterpret_cast< T* >( m_buffer )[ index ];
      }

      void clear( size_t current_capacity )
      {
        destroy( 0, current_capacity );
      }

      virtual
      void resize( size_t old_capacity, size_t new_capacity ) = 0;

      virtual
      void move( size_t from, size_t to ) = 0;

      virtual
      void destroy( size_t index ) = 0;

      virtual
      void destroy( size_t from, size_t end ) = 0;

      const size_t m_sizeof_element;
      const std::string m_name;
      unsigned char* m_buffer;
    };

    template< typename T >
    struct derived_property_buffer
      : public base_property_buffer {

      typedef T  value_type;
      typedef T* pointer_type;

      static_assert( std::is_default_constructible< value_type >::value,
         "The property is not default constructible");
      static_assert( std::is_move_assignable< value_type >::value,
         "The property is not move assignable");


      derived_property_buffer( const std::string& name )
        : base_property_buffer( sizeof( value_type ), name )
      {}

      void destroy( size_t index ) override
      {
        reinterpret_cast< pointer_type >( base_property_buffer::m_buffer )[ index ] = std::move( value_type() );
      }

      void destroy( size_t from, size_t end ) override
      {
        auto start = reinterpret_cast< pointer_type >( base_property_buffer::m_buffer ) + from,
            last = reinterpret_cast< pointer_type >( base_property_buffer::m_buffer ) + end;
        while( start < last )
          {
            *start = std::move( value_type() );
            ++start;
          }
      }

      void resize( size_t old_capacity, size_t new_capacity ) override
      {
        if( old_capacity )
          {
            auto new_buffer = new value_type[ new_capacity ];
            std::memcpy( (void*)new_buffer, (void*)base_property_buffer::m_buffer, sizeof( value_type ) * old_capacity );
            delete[] reinterpret_cast< pointer_type >( base_property_buffer::m_buffer );
            base_property_buffer::m_buffer = reinterpret_cast< unsigned char* >( new_buffer );
          }
        else
          {
            ///fixme: why do we need to handle this case separately in Release mode?
            base_property_buffer::m_buffer = reinterpret_cast< unsigned char* >( new value_type[ new_capacity ] );
          }
      }

      void move( size_t from, size_t to ) override
      {
        // copy to destination thanks to a move operator
        reinterpret_cast< pointer_type >( base_property_buffer::m_buffer )[ to ] =
            std::move( reinterpret_cast< pointer_type >( base_property_buffer::m_buffer )[ from ] );
        // put the source into a default state thanks to the default constructor
        reinterpret_cast< pointer_type >( base_property_buffer::m_buffer )[ from ] = std::move( value_type() );
      }

      ~derived_property_buffer()
      {
        delete[] reinterpret_cast<T*>( base_property_buffer::m_buffer );
      }
    };

    skeleton_datastructure( atom_handle_type nb_atoms = 0, link_handle_type nb_links = 0, face_handle_type nb_faces = 0 )
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

    ~skeleton_datastructure()
    {
      delete[] m_atoms;
      delete[] m_atom_index_to_handle_index;
      delete[] m_atom_handles;
      for( auto& property : m_atom_properties )
        delete property;

      delete[] m_links;
      delete[] m_link_index_to_handle_index;
      delete[] m_link_handles;
      for( auto& property : m_link_properties )
        delete property;

      delete[] m_faces;
      delete[] m_face_index_to_handle_index;
      delete[] m_face_handles;
      for( auto& property : m_face_properties )
        delete property;
    }

    void clear(
        atom_handle_type atom_capacity,
        link_handle_type link_capacity,
        face_handle_type face_capacity )
    {
      clear_atoms( atom_capacity );
      clear_links( link_capacity );
      clear_faces( face_capacity );
    }

    void clear_atoms( atom_handle_type atom_capacity );
    void clear_links( link_handle_type link_capacity );
    void clear_faces( face_handle_type face_capacity );

    std::pair< atom_handle, atom& > create_atom();
    std::pair< link_handle, link& > create_link();
    std::pair< face_handle, face& > create_face();

    void grow_atoms( atom_handle_type new_capacity );
    void grow_links( link_handle_type new_capacity );
    void grow_faces( face_handle_type new_capacity );

    void remove( atom_handle h );
    void remove( link_handle h );
    void remove( face_handle h );

    void remove( atom& e );
    void remove( link& e );
    void remove( face& e );

    void remove_atom_by_index( atom_handle_type index );
    void remove_link_by_index( link_handle_type index );
    void remove_face_by_index( face_handle_type index );

    atom& get( atom_handle h );
    link& get( link_handle h );
    face& get( face_handle h );

    atom& get_atom_by_index( size_t index )
    {
# ifndef MP_SKELETON_NO_CHECK
      if( index >= m_atoms_size )
        MP_THROW_EXCEPTION( skeleton_invalid_atom_index );
# endif
      return m_atoms[ index ];
    }

    link& get_link_by_index( size_t index )
    {
# ifndef MP_SKELETON_NO_CHECK
      if( index >= m_links_size )
        MP_THROW_EXCEPTION( skeleton_invalid_link_index );
# endif
      return m_links[ index ];
    }

    face& get_face_by_index( size_t index )
    {
# ifndef MP_SKELETON_NO_CHECK
      if( index >= m_faces_size )
        MP_THROW_EXCEPTION( skeleton_invalid_face_index );
# endif
      return m_faces[ index ];
    }

    atom_handle get_atom_handle( size_t index )
    {
# ifndef MP_SKELETON_NO_CHECK
      if( index >= m_atoms_size )
        MP_THROW_EXCEPTION( skeleton_invalid_atom_index );
# endif
      auto handle_index = m_atom_index_to_handle_index[ index ];
      return atom_handle( handle_index, m_atom_handles[ handle_index ].counter );
    }

    link_handle get_link_handle( size_t index )
    {
# ifndef MP_SKELETON_NO_CHECK
      if( index >= m_links_size )
        MP_THROW_EXCEPTION( skeleton_invalid_link_index );
# endif
      auto handle_index = m_link_index_to_handle_index[ index ];
      return link_handle( handle_index, m_link_handles[ handle_index ].counter );
    }

    face_handle get_face_handle( size_t index )
    {
# ifndef MP_SKELETON_NO_CHECK
      if( index >= m_faces_size )
        MP_THROW_EXCEPTION( skeleton_invalid_face_index );
# endif
      auto handle_index = m_face_index_to_handle_index[ index ];
      return face_handle( handle_index, m_face_handles[ handle_index ].counter );
    }

    atom_handle_type get_index( atom_handle handle );
    link_handle_type get_index( link_handle handle );
    face_handle_type get_index( face_handle handle );

    atom_handle_type get_index( atom& e );
    link_handle_type get_index( link& e );
    face_handle_type get_index( face& e );

    atom_handle get_handle( atom& e );
    link_handle get_handle( link& e );
    face_handle get_handle( face& e );

    template< typename T >
    void add_atom_property( const std::string& name )
    {
      m_atom_properties.push_back( new derived_property_buffer<T>( name ) );
      m_atom_properties.back()->resize( 0, m_atoms_capacity );
    }

    template< typename T >
    void add_link_property( const std::string& name )
    {
      m_link_properties.push_back( new derived_property_buffer<T>( name ) );
      m_link_properties.back()->resize( 0, m_links_capacity );
    }

    template< typename T >
    void add_face_property( const std::string& name )
    {
      m_face_properties.push_back( new derived_property_buffer<T>( name ) );
      m_face_properties.back()->resize( 0, m_faces_capacity );
    }

    atom* m_atoms;
    atom_handle_type* m_atom_index_to_handle_index;
    atom_handle_entry* m_atom_handles;
    std::vector< base_property_buffer* > m_atom_properties;

    link* m_links;
    size_t* m_link_index_to_handle_index;
    link_handle_entry* m_link_handles;
    std::vector< base_property_buffer* > m_link_properties;

    face* m_faces;
    size_t* m_face_index_to_handle_index;
    face_handle_entry* m_face_handles;
    std::vector< base_property_buffer* > m_face_properties;

    atom_handle_type m_atoms_capacity;
    atom_handle_type m_atoms_size;
    atom_handle_type m_atoms_next_free_handle_slot;

    link_handle_type m_links_capacity;
    link_handle_type m_links_size;
    link_handle_type m_links_next_free_handle_slot;

    face_handle_type m_faces_capacity;
    face_handle_type m_faces_size;
    face_handle_type m_faces_next_free_handle_slot;
  };

  template<
    typename atom_handle_type, uint8_t atom_handle_index_bits,
    typename link_handle_type, uint8_t link_handle_index_bits,
    typename face_handle_type, uint8_t face_handle_index_bits >
  constexpr uint8_t skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::atom_handle_bits;

  template<
    typename atom_handle_type, uint8_t atom_handle_index_bits,
    typename link_handle_type, uint8_t link_handle_index_bits,
    typename face_handle_type, uint8_t face_handle_index_bits >
  constexpr atom_handle_type skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::max_atom_handle_index;

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  constexpr atom_handle_type skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::max_atom_handle_counter;

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  constexpr uint8_t skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::link_handle_bits;

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  constexpr link_handle_type skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::max_link_handle_index;

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  constexpr link_handle_type skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::max_link_handle_counter;

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  constexpr uint8_t skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::face_handle_bits;

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  constexpr face_handle_type skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::max_face_handle_index;

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  constexpr face_handle_type skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::max_face_handle_counter;

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  void skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::clear_atoms( atom_handle_type atom_capacity )
  {
    if( atom_capacity < m_atoms_capacity )
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
        delete[] m_atoms;
        delete[] m_atom_handles;
        delete[] m_atom_index_to_handle_index;

        m_atoms = nullptr;
        m_atom_handles = nullptr;
        m_atom_index_to_handle_index = nullptr;

        m_atoms_capacity = 0;
        m_atoms_size = 0;
        m_atoms_next_free_handle_slot = 0;

        grow_atoms( atom_capacity );
      }
  }

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  void skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::clear_links( link_handle_type link_capacity )
  {
    if( link_capacity < m_links_capacity )
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
        delete[] m_links;
        delete[] m_link_handles;
        delete[] m_link_index_to_handle_index;

        m_links = nullptr;
        m_link_handles = nullptr;
        m_link_index_to_handle_index = nullptr;

        m_links_capacity = 0;
        m_links_size = 0;
        m_links_next_free_handle_slot = 0;

        grow_links( link_capacity );
      }
  }

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  void skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::clear_faces( face_handle_type face_capacity )
  {
    if( face_capacity < m_faces_capacity )
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
        delete[] m_faces;
        delete[] m_face_handles;
        delete[] m_face_index_to_handle_index;

        m_faces = nullptr;
        m_face_handles = nullptr;
        m_face_index_to_handle_index = nullptr;

        m_faces_capacity = 0;
        m_faces_size = 0;
        m_faces_next_free_handle_slot = 0;

        grow_faces( face_capacity );
      }
  }

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  void skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::grow_atoms( atom_handle_type new_capacity )
  {
    // check for overflow
    if( new_capacity <= m_atoms_capacity || new_capacity > max_atom_handle_index )
      MP_THROW_EXCEPTION( skeleton_atom_buffer_overflow );

    // initialize data
    if( !m_atoms )
      {
        m_atoms = new atom[ new_capacity ];
        m_atom_index_to_handle_index = new atom_handle_type[ new_capacity ];
        m_atom_handles = new atom_handle_entry[ new_capacity ];
      }
    // resize buffers
    else
      {
        auto new_element_buffer = new atom[ new_capacity ];
        auto new_element_to_handle = new atom_handle_type[ new_capacity ];
        auto new_handle_buffer = new atom_handle_entry[ new_capacity ];
        # pragma omp parallel for
        for( atom_handle_type i = 0; i < m_atoms_capacity; ++ i )
          {
            new_element_buffer[ i ] = std::move( m_atoms[ i ] );
            new_element_to_handle[ i ] = m_atom_index_to_handle_index[ i ];
            new_handle_buffer[ i ] = m_atom_handles[ i ];
          }
        delete[] m_atoms;
        delete[] m_atom_index_to_handle_index;
        delete[] m_atom_handles;

        m_atoms = new_element_buffer;
        m_atom_index_to_handle_index = new_element_to_handle;
        m_atom_handles = new_handle_buffer;
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

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  void skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::grow_links( link_handle_type new_capacity )
  {
    // check for overflow
    if( new_capacity <= m_links_capacity || new_capacity > max_link_handle_index )
      MP_THROW_EXCEPTION( skeleton_link_buffer_overflow );

    // initialize data
    if( !m_links )
      {
        m_links = new link[ new_capacity ];
        m_link_index_to_handle_index = new size_t[ new_capacity ];
        m_link_handles = new link_handle_entry[ new_capacity ];

//        add_link_property<std::vector<face_handle>>( "faces" );
      }
    // resize buffers
    else
      {
        auto new_element_buffer = new link[ new_capacity ];
        auto new_element_to_handle = new size_t[ new_capacity ];
        auto new_handle_buffer = new link_handle_entry[ new_capacity ];
        # pragma omp parallel for
        for( size_t i = 0; i < m_links_capacity; ++ i )
          {
            new_element_buffer[ i ] = std::move( m_links[ i ] );
            new_element_to_handle[ i ] = m_link_index_to_handle_index[ i ];
            new_handle_buffer[ i ] = m_link_handles[ i ];
          }
        delete[] m_links;
        delete[] m_link_index_to_handle_index;
        delete[] m_link_handles;

        m_links = new_element_buffer;
        m_link_index_to_handle_index = new_element_to_handle;
        m_link_handles = new_handle_buffer;
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

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  void skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::grow_faces( face_handle_type new_capacity )
  {
    // check for overflow
    if( new_capacity <= m_faces_capacity || new_capacity > max_face_handle_index )
      MP_THROW_EXCEPTION( skeleton_face_buffer_overflow );

    // initialize data
    if( !m_faces )
      {
        m_faces = new face[ new_capacity ];
        m_face_index_to_handle_index = new size_t[ new_capacity ];
        m_face_handles = new face_handle_entry[ new_capacity ];
      }
    // resize buffers
    else
      {
        auto new_element_buffer = new face[ new_capacity ];
        auto new_element_to_handle = new size_t[ new_capacity ];
        auto new_handle_buffer = new face_handle_entry[ new_capacity ];
        # pragma omp parallel for
        for( size_t i = 0; i < m_faces_capacity; ++ i )
          {
            new_element_buffer[ i ] = std::move( m_faces[ i ] );
            new_element_to_handle[ i ] = m_face_index_to_handle_index[ i ];
            new_handle_buffer[ i ] = m_face_handles[ i ];
          }
        delete[] m_faces;
        delete[] m_face_index_to_handle_index;
        delete[] m_face_handles;

        m_faces = new_element_buffer;
        m_face_index_to_handle_index = new_element_to_handle;
        m_face_handles = new_handle_buffer;
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


  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  void skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::remove( atom_handle h )
  {
# ifndef MP_SKELETON_NO_CHECK
    if( h.index >= m_atoms_capacity )
      MP_THROW_EXCEPTION( skeleton_invalid_atom_handle );
# endif
    auto entry = m_atom_handles + h.index;
    if( entry->status != STATUS_ALLOCATED )
      return;
# ifndef MP_SKELETON_NO_CHECK
    if( entry->counter != h.counter )
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

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  void skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::remove( link_handle h )
  {
# ifndef MP_SKELETON_NO_CHECK
    if( h.index >= m_links_capacity )
      MP_THROW_EXCEPTION( skeleton_invalid_link_handle );
# endif
    auto entry = m_link_handles + h.index;
    if( entry->status != STATUS_ALLOCATED )
      return;
# ifndef MP_SKELETON_NO_CHECK
    if( entry->counter != h.counter )
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
        m_links[index].~link(); // not even necessary
        for( auto& property : m_link_properties )
          property->destroy( index );
      }
  }

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  void skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::remove( face_handle h )
  {
# ifndef MP_SKELETON_NO_CHECK
    if( h.index >= m_faces_capacity )
      MP_THROW_EXCEPTION( skeleton_invalid_face_handle );
# endif
    auto entry = m_face_handles + h.index;
    if( entry->status != STATUS_ALLOCATED )
      return;
# ifndef MP_SKELETON_NO_CHECK
    if( entry->counter != h.counter )
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
        m_faces[index].~face(); // not even necessary
        for( auto& property : m_face_properties )
          property->destroy( index );
      }
  }

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  void skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::remove_atom_by_index( atom_handle_type index )
  {
# ifndef MP_SKELETON_NO_CHECK
      if( index >= m_atoms_size )
        MP_THROW_EXCEPTION( skeleton_invalid_atom_index );
# endif
      // update the entry
      {
        const auto entry_index = m_atom_index_to_handle_index[ index ];
        auto entry = m_atom_handles + entry_index;
        if( entry->status == STATUS_FREE )
          return;
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
          m_atoms[index].~atom(); // not even necessary
          for( auto& property : m_atom_properties )
            property->destroy( index );
        }
  }

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  void skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::remove_link_by_index( link_handle_type index )
  {
# ifndef MP_SKELETON_NO_CHECK
      if( index >= m_links_size )
        MP_THROW_EXCEPTION( skeleton_invalid_link_index );
# endif
      // update the entry
      {
        const auto entry_index = m_link_index_to_handle_index[ index ];
        auto entry = m_link_handles + entry_index;
        if( entry->status == STATUS_FREE )
          return;
        entry->next_free_index = m_links_next_free_handle_slot;
        entry->status = STATUS_FREE;
        m_links_next_free_handle_slot = entry_index;
      }

      // move the last link into this one to keep the buffer tight
      if( --m_links_size && index != m_links_size )
        {
          // move the link itself
          m_links[ index ] = std::move( m_links[ m_links_size ] );

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
          m_links[index].~link(); // not even necessary
          for( auto& property : m_link_properties )
            property->destroy( index );
        }
  }

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  void skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::remove_face_by_index( face_handle_type index )
  {
# ifndef MP_SKELETON_NO_CHECK
      if( index >= m_faces_size )
        MP_THROW_EXCEPTION( skeleton_invalid_face_index );
# endif
      // update the entry
      {
        const auto entry_index = m_face_index_to_handle_index[ index ];
        auto entry = m_face_handles + entry_index;
        if( entry->status == STATUS_FREE )
          return;
        entry->next_free_index = m_faces_next_free_handle_slot;
        entry->status = STATUS_FREE;
        m_faces_next_free_handle_slot = entry_index;
      }

      // move the last face into this one to keep the buffer tight
      if( --m_faces_size && index != m_faces_size )
        {
          // move the face itself
          m_faces[ index ] = std::move( m_faces[ m_faces_size ] );

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
          m_faces[index].~face(); // not even necessary
          for( auto& property : m_face_properties )
            property->destroy( index );
        }
  }

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  void skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::remove( atom& e )
  {
# ifndef MP_SKELETON_NO_CHECK
    if( &e < m_atoms || &e >= m_atoms + m_atoms_size )
      MP_THROW_EXCEPTION( skeleton_invalid_atom_pointer );
# endif
    const atom_handle_type index = std::distance( m_atoms, &e );
# ifndef MP_SKELETON_NO_CHECK
    if( m_atoms + index != &e )
      MP_THROW_EXCEPTION( skeleton_invalid_atom_pointer );
# endif

    // update the entry
    {
      const auto entry_index = m_atom_index_to_handle_index[ index ];
      auto entry = m_atom_handles + entry_index;
      if( entry->status == STATUS_FREE )
        return;
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
        e.~atom(); // not even necessary
        for( auto& property : m_atom_properties )
          property->destroy( index );
      }
  }

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  void skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::remove( link& e )
  {
# ifndef MP_SKELETON_NO_CHECK
    if( &e < m_links || &e >= m_links + m_links_size )
      MP_THROW_EXCEPTION( skeleton_invalid_link_pointer );
# endif
    const link_handle_type index = std::distance( m_links, &e );
# ifndef MP_SKELETON_NO_CHECK
    if( m_links + index != &e )
      MP_THROW_EXCEPTION( skeleton_invalid_link_pointer );
# endif

    // update the entry
    {
      const auto entry_index = m_link_index_to_handle_index[ index ];
      auto entry = m_link_handles + entry_index;
      if( entry->status == STATUS_FREE )
        return;
      entry->next_free_index = m_links_next_free_handle_slot;
      entry->status = STATUS_FREE;
      m_links_next_free_handle_slot = entry_index;
    }

    // move the last link into this one to keep the buffer tight
    if( --m_links_size && index != m_links_size )
      {
        // move the link itself
        e = std::move( m_links[ m_links_size ] );

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
        e.~link(); // not even necessary
        for( auto& property : m_link_properties )
          property->destroy( index );
      }
  }

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  void skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::remove( face& e )
  {
# ifndef MP_SKELETON_NO_CHECK
    if( &e < m_faces || &e >= m_faces + m_faces_size )
      MP_THROW_EXCEPTION( skeleton_invalid_face_pointer );
# endif
    const face_handle_type index = std::distance( m_faces, &e );
# ifndef MP_SKELETON_NO_CHECK
    if( m_faces + index != &e )
      MP_THROW_EXCEPTION( skeleton_invalid_face_pointer );
# endif

    // update the entry
    {
      const auto entry_index = m_face_index_to_handle_index[ index ];
      auto entry = m_face_handles + entry_index;
      if( entry->status == STATUS_FREE )
        return;
      entry->next_free_index = m_faces_next_free_handle_slot;
      entry->status = STATUS_FREE;
      m_faces_next_free_handle_slot = entry_index;
    }

    // move the last face into this one to keep the buffer tight
    if( --m_faces_size && index != m_faces_size )
      {
        // move the face itself
        e = std::move( m_faces[ m_faces_size ] );

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
        e.~face(); // not even necessary
        for( auto& property : m_face_properties )
          property->destroy( index );
      }
  }


  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  typename skeleton_datastructure<
      atom_handle_type, atom_handle_index_bits,
      link_handle_type, link_handle_index_bits,
      face_handle_type, face_handle_index_bits>::atom&
  skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::get( atom_handle h )
  {
# ifndef MP_SKELETON_NO_CHECK
    if( h.index >= m_atoms_capacity )
      MP_THROW_EXCEPTION( skeleton_invalid_atom_handle );
# endif
    const auto entry = m_atom_handles + h.index;
# ifndef MP_SKELETON_NO_CHECK
    if( entry->status != STATUS_ALLOCATED || entry->counter != h.counter )
      MP_THROW_EXCEPTION( skeleton_invalid_atom_handle );
# endif
    return m_atoms[ entry->atom_index ];
  }

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  typename skeleton_datastructure<
      atom_handle_type, atom_handle_index_bits,
      link_handle_type, link_handle_index_bits,
      face_handle_type, face_handle_index_bits>::link&
  skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::get( link_handle h )
  {
# ifndef MP_SKELETON_NO_CHECK
    if( h.index >= m_links_capacity )
      MP_THROW_EXCEPTION( skeleton_invalid_link_handle );
# endif
    const auto entry = m_link_handles + h.index;
# ifndef MP_SKELETON_NO_CHECK
    if( entry->status != STATUS_ALLOCATED || entry->counter != h.counter )
      MP_THROW_EXCEPTION( skeleton_invalid_link_handle );
# endif
    return m_links[ entry->link_index ];
  }

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  typename skeleton_datastructure<
      atom_handle_type, atom_handle_index_bits,
      link_handle_type, link_handle_index_bits,
      face_handle_type, face_handle_index_bits>::face&
  skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::get( face_handle h )
  {
# ifndef MP_SKELETON_NO_CHECK
    if( h.index >= m_faces_capacity )
      MP_THROW_EXCEPTION( skeleton_invalid_face_handle );
# endif
    const auto entry = m_face_handles + h.index;
# ifndef MP_SKELETON_NO_CHECK
    if( entry->status != STATUS_ALLOCATED || entry->counter != h.counter )
      MP_THROW_EXCEPTION( skeleton_invalid_face_handle );
# endif
    return m_faces[ entry->face_index ];
  }

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  atom_handle_type
  skeleton_datastructure<
      atom_handle_type, atom_handle_index_bits,
      link_handle_type, link_handle_index_bits,
      face_handle_type, face_handle_index_bits>::get_index( atom_handle handle )
  {
# ifndef MP_SKELETON_NO_CHECK
    if( handle.index >= m_atoms_capacity )
      MP_THROW_EXCEPTION( skeleton_invalid_atom_handle );
# endif
    const auto entry = m_atom_handles + handle.index;
# ifndef MP_SKELETON_NO_CHECK
    if( entry->status != STATUS_ALLOCATED || entry->counter != handle.counter )
      MP_THROW_EXCEPTION( skeleton_invalid_atom_handle );
# endif
    return entry->atom_index;
  }

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  link_handle_type
  skeleton_datastructure<
      atom_handle_type, atom_handle_index_bits,
      link_handle_type, link_handle_index_bits,
      face_handle_type, face_handle_index_bits>::get_index( link_handle handle )
  {
# ifndef MP_SKELETON_NO_CHECK
    if( handle.index >= m_links_capacity )
      MP_THROW_EXCEPTION( skeleton_invalid_link_handle );
# endif
    const auto entry = m_link_handles + handle.index;
# ifndef MP_SKELETON_NO_CHECK
    if( entry->status != STATUS_ALLOCATED || entry->counter != handle.counter )
      MP_THROW_EXCEPTION( skeleton_invalid_link_handle );
# endif
    return entry->link_index;
  }

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  face_handle_type
  skeleton_datastructure<
      atom_handle_type, atom_handle_index_bits,
      link_handle_type, link_handle_index_bits,
      face_handle_type, face_handle_index_bits>::get_index( face_handle handle )
  {
# ifndef MP_SKELETON_NO_CHECK
    if( handle.index >= m_faces_capacity )
      MP_THROW_EXCEPTION( skeleton_invalid_face_handle );
# endif
    const auto entry = m_face_handles + handle.index;
# ifndef MP_SKELETON_NO_CHECK
    if( entry->status != STATUS_ALLOCATED || entry->counter != handle.counter )
      MP_THROW_EXCEPTION( skeleton_invalid_face_handle );
# endif
    return entry->face_index;
  }

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  typename skeleton_datastructure<
      atom_handle_type, atom_handle_index_bits,
      link_handle_type, link_handle_index_bits,
      face_handle_type, face_handle_index_bits>::atom_handle
  skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::get_handle( atom& e )
  {
# ifndef MP_SKELETON_NO_CHECK
    if( &e < m_atoms || &e >= m_atoms + m_atoms_size )
      MP_THROW_EXCEPTION( skeleton_invalid_atom_pointer );
# endif
    const auto element_index = std::distance( m_atoms, &e );
# ifndef MP_SKELETON_NO_CHECK
    if( m_atoms + element_index != &e )
      MP_THROW_EXCEPTION( skeleton_invalid_atom_pointer );
# endif
    auto idx = m_atom_index_to_handle_index[ element_index ];
    return atom_handle( idx, m_atom_handles[ idx ].counter );
  }

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  typename skeleton_datastructure<
      atom_handle_type, atom_handle_index_bits,
      link_handle_type, link_handle_index_bits,
      face_handle_type, face_handle_index_bits>::link_handle
  skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::get_handle( link& e )
  {
# ifndef MP_SKELETON_NO_CHECK
    if( &e < m_links || &e >= m_links + m_links_size )
      MP_THROW_EXCEPTION( skeleton_invalid_link_pointer );
# endif
    const auto element_index = std::distance( m_links, &e );
# ifndef MP_SKELETON_NO_CHECK
    if( m_links + element_index != &e )
      MP_THROW_EXCEPTION( skeleton_invalid_link_pointer );
# endif
    auto idx = m_link_index_to_handle_index[ element_index ];
    return link_handle( idx, m_link_handles[ idx ].counter );
  }

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  typename skeleton_datastructure<
      atom_handle_type, atom_handle_index_bits,
      link_handle_type, link_handle_index_bits,
      face_handle_type, face_handle_index_bits>::face_handle
  skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::get_handle( face& e )
  {
# ifndef MP_SKELETON_NO_CHECK
    if( &e < m_faces || &e >= m_faces + m_faces_size )
      MP_THROW_EXCEPTION( skeleton_invalid_face_pointer );
# endif
    const auto element_index = std::distance( m_faces, &e );
# ifndef MP_SKELETON_NO_CHECK
    if( m_faces + element_index != &e )
      MP_THROW_EXCEPTION( skeleton_invalid_face_pointer );
# endif
    auto idx = m_face_index_to_handle_index[ element_index ];
    return face_handle( idx, m_face_handles[ idx ].counter );
  }

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  atom_handle_type skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::get_index( atom& e )
  {
# ifndef MP_SKELETON_NO_CHECK
    if( &e < m_atoms || &e >= m_atoms + m_atoms_size )
      MP_THROW_EXCEPTION( skeleton_invalid_atom_pointer );
# endif
    const auto element_index = std::distance( m_atoms, &e );
# ifndef MP_SKELETON_NO_CHECK
    if( m_atoms + element_index != &e )
      MP_THROW_EXCEPTION( skeleton_invalid_atom_pointer );
# endif
    return element_index;
  }

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  link_handle_type skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::get_index( link& e )
  {
# ifndef MP_SKELETON_NO_CHECK
    if( &e < m_links || &e >= m_links + m_links_size )
      MP_THROW_EXCEPTION( skeleton_invalid_link_pointer );
# endif
    const auto element_index = std::distance( m_links, &e );
# ifndef MP_SKELETON_NO_CHECK
    if( m_links + element_index != &e )
      MP_THROW_EXCEPTION( skeleton_invalid_link_pointer );
# endif
    return element_index;
  }

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  face_handle_type skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::get_index( face& e )
  {
# ifndef MP_SKELETON_NO_CHECK
    if( &e < m_faces || &e >= m_faces + m_faces_size )
      MP_THROW_EXCEPTION( skeleton_invalid_face_pointer );
# endif
    const auto element_index = std::distance( m_faces, &e );
# ifndef MP_SKELETON_NO_CHECK
    if( m_faces + element_index != &e )
      MP_THROW_EXCEPTION( skeleton_invalid_face_pointer );
# endif
    return element_index;
  }
END_MP_NAMESPACE
# endif 
