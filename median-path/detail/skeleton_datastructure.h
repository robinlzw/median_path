/* Created on: Mar 1, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# ifndef MEDIAN_PATH_SKELETON_DATASTRUCTURE_H_
# define MEDIAN_PATH_SKELETON_DATASTRUCTURE_H_

# include "median_path.h"
# include "detail/exceptions.h"

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
   * link structure.
   *
   * - faces
   * I consider only triangular faces. Some structuration algorithms produce
   * faces that are not triangular, but there is currently no need for non
   * triangular faces. Thus a face connects three vertices and three links.
   * So an atom must know about its faces, a link must also know about its
   * faces: those information are respectively atom data and link data.
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
    static constexpr atom_handle_type max_atom_handle_index = ( 1 << atom_handle_index_bits ) - 1;
    static constexpr atom_handle_type max_atom_handle_counter = ( 1 << (atom_handle_bits - atom_handle_index_bits - 2 )) - 1;

    static constexpr uint8_t link_handle_bits = sizeof( link_handle_type ) << 3;
    static constexpr link_handle_type max_link_handle_index = ( 1 << link_handle_index_bits ) - 1;
    static constexpr link_handle_type max_link_handle_counter = ( 1 << (link_handle_bits - link_handle_index_bits - 2 )) - 1;

    static constexpr uint8_t face_handle_bits = sizeof( face_handle_type ) << 3;
    static constexpr face_handle_type max_face_handle_index = ( 1 << face_handle_index_bits ) - 1;
    static constexpr face_handle_type max_face_handle_counter = ( 1 << (face_handle_bits - face_handle_index_bits - 2 )) - 1;

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

      size_t atom_index;

      atom_handle_entry()
        : next_free_index{ 0 }, counter{ 0 },
          status{ STATUS_FREE }, atom_index{ 0 }
      {}
    };
    struct link_handle_entry {
      link_handle_type next_free_index : link_handle_index_bits;
      link_handle_type counter         : link_handle_bits - link_handle_index_bits - 2;
      link_handle_type status          : 2;

      size_t link_index;

      link_handle_entry()
        : next_free_index{ 0 }, counter{ 0 },
          status{ STATUS_FREE }, link_index{ 0 }
      {}
    };
    struct face_handle_entry {
      face_handle_type next_free_index : face_handle_index_bits;
      face_handle_type counter         : face_handle_bits - face_handle_index_bits - 2;
      face_handle_type status          : 2;

      size_t face_index;

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
      inline operator atom_handle_type() const
      {
        return (counter << atom_handle_index_bits ) | index;
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
        : index{ max_link_handle_index },
          counter{ max_link_handle_counter + 1 }
      {}
      inline operator link_handle_type() const
      {
        return (counter << link_handle_index_bits ) | index;
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
          counter{ max_face_handle_counter + 1 }
      {}
      inline operator face_handle_type() const
      {
        return (counter << face_handle_index_bits ) | index;
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
     *  - two virtual functions to be able to call the destructor of the      *
     *  actual types: a) the destructor and b) a resize function.             *
     **************************************************************************/
    struct base_property {
      base_property( size_t size, const std::string& name )
        : m_sizeof_element{ size }, m_name{ name },
          m_buffer{ nullptr }
      {}

      virtual ~base_property() {}

      template< typename T >
      T& get( size_t index )
      {
        return static_cast< T* >( m_buffer )[ index ];
      }

      virtual
      void resize( size_t old_capacity, size_t new_capacity ) = 0;

      const size_t m_sizeof_element;
      const std::string m_name;
      unsigned char* m_buffer;
    };

    template< typename T >
    struct derived_property
      : public base_property {

      typedef T value_type;

      derived_property( const std::string& name )
        : base_property( sizeof( T ), name )
      {}

      void resize( size_t old_capacity, size_t new_capacity )
      {
        auto new_buffer = new value_type[ new_capacity ];
        std::memcpy( (void*)new_buffer, (void*)base_property::m_buffer, sizeof( value_type ) * old_capacity );
        base_property::m_buffer = reinterpret_cast< unsigned char* >( new_buffer );
      }

      void clear( size_t current_capacity )
      {
        auto ptr = reinterpret_cast< T* >( base_property::m_buffer );
        for( size_t i = 0; i < current_capacity; ++ i, ++ ptr )
          {
            *ptr = T();
          }
      }

      ~derived_property()
      {
        delete[] reinterpret_cast<T*>( base_property::m_buffer );
      }
    };

    skeleton_datastructure( size_t nb_atoms = 0, size_t nb_links = 0, size_t nb_faces = 0 )
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

    }

    void clear_atoms( atom_handle_type atom_capacity );
    void clear_links( link_handle_type link_capacity );
    void clear_faces( face_handle_type face_capacity );

    std::pair< atom_handle, atom& > create_atom();
    std::pair< link_handle, link& > create_link();
    std::pair< face_handle, face& > create_face();

    void grow_atoms( size_t new_capacity );
    void grow_links( size_t new_capacity );
    void grow_faces( size_t new_capacity );

    void remove( atom_handle h )
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
      entry->next_free_index = m_atoms_next_free_handle_slot;
      entry->status = STATUS_FREE;
      auto& e = m_atoms[ entry->atom_index ];
      e.~atom();
      m_atoms_next_free_handle_slot = h.index;
      if( --m_atoms_size && entry->atom_index != m_atoms_size )
        {
          m_atoms[ entry->atom_index ] = std::move( m_atoms[ m_atoms_size ] );
          m_atom_handles[ m_atom_index_to_handle_index[ entry->atom_index ] ].atom_entry = entry->atom_index;
        }
    }

    void remove( link_handle h )
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
      entry->next_free_index = m_links_next_free_handle_slot;
      entry->status = STATUS_FREE;
      auto& e = m_links[ entry->link_index ];
      e.~link();
      m_links_next_free_handle_slot = h.index;
      if( --m_links_size && entry->link_index != m_links_size )
        {
          m_links[ entry->link_index ] = std::move( m_links[ m_links_size ] );
          m_link_handles[ m_link_index_to_handle_index[ entry->link_index ] ].link_entry = entry->link_index;
        }
    }

    void remove( face_handle h )
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
      entry->next_free_index = m_faces_next_free_handle_slot;
      entry->status = STATUS_FREE;
      auto& e = m_faces[ entry->face_index ];
      e.~face();
      m_faces_next_free_handle_slot = h.index;
      if( --m_faces_size && entry->face_index != m_faces_size )
        {
          m_faces[ entry->face_index ] = std::move( m_faces[ m_faces_size ] );
          m_face_handles[ m_face_index_to_handle_index[ entry->face_index ] ].face_entry = entry->face_index;
        }
    }

    void remove( atom& e )
    {
# ifndef MP_SKELETON_NO_CHECK
      if( e < m_atoms || e >= m_atoms + m_atoms_size )
        MP_THROW_EXCEPTION( skeleton_invalid_atom_pointer );
# endif
      const auto element_index = std::distance( m_atoms, &e );
# ifndef MP_SKELETON_NO_CHECK
      if( m_atoms + element_index != &e )
        MP_THROW_EXCEPTION( skeleton_invalid_atom_pointer );
# endif
      const auto entry_index = m_atom_index_to_handle_index[ element_index ];
      auto entry = m_atom_handles + entry_index;
      if( entry->status != STATUS_ALLOCATED )
        return;

      entry->next_free_index = m_atoms_next_free_handle_slot;
      entry->status = STATUS_FREE;
      e.~atom();
      m_atoms_next_free_handle_slot = entry_index;
      if( --m_atoms_size && entry->atom_index != m_atoms_size )
        {
          m_atoms[ entry->atom_index ] = std::move( m_atoms[ m_atoms_size ] );
          m_atom_handles[ m_atom_index_to_handle_index[ entry->atom_index ] ].atom_entry = entry->atom_index;
        }
    }

    void remove( link& e )
    {
# ifndef MP_SKELETON_NO_CHECK
      if( e < m_links || e >= m_links + m_links_size )
        MP_THROW_EXCEPTION( skeleton_invalid_link_pointer );
# endif
      const auto element_index = std::distance( m_links, &e );
# ifndef MP_SKELETON_NO_CHECK
      if( m_links + element_index != &e )
        MP_THROW_EXCEPTION( skeleton_invalid_link_pointer );
# endif
      const auto entry_index = m_link_index_to_handle_index[ element_index ];
      auto entry = m_link_handles + entry_index;
      if( entry->status != STATUS_ALLOCATED )
        return;

      entry->next_free_index = m_links_next_free_handle_slot;
      entry->status = STATUS_FREE;
      e.~link();
      m_links_next_free_handle_slot = entry_index;
      if( --m_links_size && entry->link_index != m_links_size )
        {
          m_links[ entry->link_index ] = std::move( m_links[ m_links_size ] );
          m_link_handles[ m_link_index_to_handle_index[ entry->link_index ] ].link_entry = entry->link_index;
        }
    }

    void remove( face& e )
    {
# ifndef MP_SKELETON_NO_CHECK
      if( e < m_faces || e >= m_faces + m_faces_size )
        MP_THROW_EXCEPTION( skeleton_invalid_face_pointer );
# endif
      const auto element_index = std::distance( m_faces, &e );
# ifndef MP_SKELETON_NO_CHECK
      if( m_faces + element_index != &e )
        MP_THROW_EXCEPTION( skeleton_invalid_face_pointer );
# endif
      const auto entry_index = m_face_index_to_handle_index[ element_index ];
      auto entry = m_face_handles + entry_index;
      if( entry->status != STATUS_ALLOCATED )
        return;

      entry->next_free_index = m_faces_next_free_handle_slot;
      entry->status = STATUS_FREE;
      e.~face();
      m_faces_next_free_handle_slot = entry_index;
      if( --m_faces_size && entry->face_index != m_faces_size )
        {
          m_faces[ entry->face_index ] = std::move( m_faces[ m_faces_size ] );
          m_face_handles[ m_face_index_to_handle_index[ entry->face_index ] ].face_entry = entry->face_index;
        }
    }

    atom& get( atom_handle h )
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

    link& get( link_handle h )
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

    face& get( face_handle h )
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


    template< typename T >
    void add_atom_property( const std::string& name )
    {
      m_atom_properties.push_back( new derived_property<T>( name ) );
      m_atom_properties.back()->resize( 0, m_atoms_capacity );
    }

    template< typename T >
    void add_link_property( const std::string& name )
    {
      m_link_properties.push_back( new derived_property<T>( name ) );
      m_link_properties.back()->resize( 0, m_links_capacity );
    }

    template< typename T >
    void add_face_property( const std::string& name )
    {
      m_face_properties.push_back( new derived_property<T>( name ) );
      m_face_properties.back()->resize( 0, m_faces_capacity );
    }

    atom* m_atoms;
    size_t* m_atom_index_to_handle_index;
    atom_handle_entry* m_atom_handles;
    std::vector< base_property* > m_atom_properties;

    link* m_links;
    size_t* m_link_index_to_handle_index;
    link_handle_entry* m_link_handles;
    std::vector< base_property* > m_link_properties;

    face* m_faces;
    size_t* m_face_index_to_handle_index;
    face_handle_entry* m_face_handles;
    std::vector< base_property* > m_face_properties;


    size_t m_atoms_capacity;
    size_t m_atoms_size;
    size_t m_atoms_next_free_handle_slot;

    size_t m_links_capacity;
    size_t m_links_size;
    size_t m_links_next_free_handle_slot;

    size_t m_faces_capacity;
    size_t m_faces_size;
    size_t m_faces_next_free_handle_slot;
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
    // need to
    if( atom_capacity < m_atoms_capacity )
      {
        for( auto& element : m_atoms )
          *element = atom{};



        for( auto& property : m_atom_properties )
          property->clear( m_atoms_capacity );
      }
    else
      {

      }
    m_atoms_size = 0;
  }


  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  void skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::grow_atoms( size_t new_capacity )
  {
    // check for overflow
    if( new_capacity <= m_atoms_capacity || new_capacity > max_atom_handle_index )
      MP_THROW_EXCEPTION( skeleton_atom_buffer_overflow );

    // initialize data
    if( !m_atoms )
      {
        m_atoms = new atom[ new_capacity ];
        m_atom_index_to_handle_index = new size_t[ new_capacity ];
        m_atom_handles = new atom_handle_entry[ new_capacity ];

        add_atom_property<std::vector<link_handle>>( "links" );
        add_atom_property<std::vector<face_handle>>( "faces" );
      }
    // resize buffers
    else
      {
        auto new_element_buffer = new atom[ new_capacity ];
        auto new_element_to_handle = new size_t[ new_capacity ];
        auto new_handle_buffer = new atom_handle_entry[ new_capacity ];
        # pragma omp parallel for
        for( size_t i = 0; i < m_atoms_capacity; ++ i )
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
    face_handle_type, face_handle_index_bits>::grow_links( size_t new_capacity )
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
        m_link_properties[i]->template resize( m_links_capacity, new_capacity );
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
    face_handle_type, face_handle_index_bits>::grow_faces( size_t new_capacity )
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
        m_face_properties[i]->template resize( m_faces_capacity, new_capacity );
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
          m_atoms_capacity + std::max( m_atoms_capacity, size_t{10} ) ) );
      /* fetch the handle entry */
      const auto handle_index = m_atoms_next_free_handle_slot;
      auto entry = m_atom_handles + handle_index;
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
      m_atoms_next_free_handle_slot = entry->next_free_index;
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
        m_links_capacity + std::max( m_links_capacity, size_t{10} ) ) );
    /* fetch the handle entry */
    const auto handle_index = m_links_next_free_handle_slot;
    auto entry = m_link_handles + handle_index;
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
    m_links_next_free_handle_slot = entry->next_free_index;
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
        m_faces_capacity + std::max( m_faces_capacity, size_t{10} ) ) );
    /* fetch the handle entry */
    const auto handle_index = m_faces_next_free_handle_slot;
    auto entry = m_face_handles + handle_index;
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
    m_faces_next_free_handle_slot = entry->next_free_index;
    ++m_faces_size;
    return result;
  }
END_MP_NAMESPACE
# endif 
