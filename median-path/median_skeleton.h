/* Created on: Mar 1, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# ifndef MEDIAN_PATH_MEDIAN_SKELETON_H_
# define MEDIAN_PATH_MEDIAN_SKELETON_H_

# include "median_path.h"
# include "detail/exceptions.h"

# include <graphics-origin/geometry/vec.h>

# include <type_traits>

BEGIN_MP_NAMESPACE

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

  };

  struct face {

  };

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
   * a link data in a tight buffer).
   *
   * - faces
   * I consider only triangular faces. Some structuration algorithms produce
   * faces that are not triangular, but there is currently no need for non
   * triangular faces. Thus a face connects three vertices and three links.
   * So an atom must know about its faces, a link must also know about its
   * faces: those information are respectively atom data and link data.
   *
   * atom* atom_buffer;
   * atom_handle_entry* atom_handle_buffer;
   * std::vector<atom_handle>* link_buffer;
   *
   * link { // structure generated in iterators/processors
   *   atom_handle to;
   *   link_handle self;
   * }
   *
   * face {
   *   atom_handle atoms[3];
   *   link_handle links[3];
   * }
   *
   *
   */
  template<
    typename atom_handle_type, uint8_t atom_handle_index_bits,
    typename link_handle_type, uint8_t link_handle_index_bits,
    typename face_handle_type, uint8_t face_handle_index_bits >
  struct skeleton_datastructure {

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
    static constexpr size_t max_atom_handle_index = ( 1 << atom_handle_index_bits ) - 1;
    static constexpr size_t max_atom_handle_counter = ( 1 << (atom_handle_bits - atom_handle_index_bits - 2 )) - 1;

    static constexpr uint8_t link_handle_bits = sizeof( link_handle_type ) << 3;
    static constexpr size_t max_link_handle_index = ( 1 << link_handle_index_bits ) - 1;
    static constexpr size_t max_link_handle_counter = ( 1 << (link_handle_bits - link_handle_index_bits - 2 )) - 1;

    static constexpr uint8_t face_handle_bits = sizeof( face_handle_type ) << 3;
    static constexpr size_t max_face_handle_index = ( 1 << face_handle_index_bits ) - 1;
    static constexpr size_t max_face_handle_counter = ( 1 << (face_handle_bits - face_handle_index_bits - 2 )) - 1;

    enum{ STATUS_FREE = 0, STATUS_ALLOCATED = 1, STATUS_GARBAGE = 2 };

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

      delete[] m_links;
      delete[] m_link_index_to_handle_index;
      delete[] m_link_handles;

      delete[] m_faces;
      delete[] m_face_index_to_handle_index;
      delete[] m_face_handles;
    }

# define CREATE_ELEMENT( name )                                                \
  /* resize if needed */                                                       \
  if( m_##name##s_size == m_##name##s_capacity )                               \
    grow_##name##s( std::min(                                                  \
      max_##name##_handle_index,                                               \
      m_##name##s_capacity + std::max( m_##name##s_capacity, size_t{10} ) ) ); \
  /* fetch the handle entry */                                                 \
  const auto handle_index = m_##name##s_next_free_handle_slot;                 \
  auto entry = m_##name##_handles + handle_index;                              \
  /* update the entry */                                                       \
  ++entry->counter;                                                            \
  if( entry->counter > max_##name##_handle_counter )                           \
    entry->counter = 0;                                                        \
  entry->name##_index = m_##name##s_size;                                      \
  entry->next_free_index = 0;                                                  \
  entry->status = STATUS_ALLOCATED;                                            \
  /* map the element index to the handle entry */                              \
  m_##name##_index_to_handle_index[ m_##name##s_size ] = handle_index;         \
  /* prepare the result */                                                     \
  std::pair< name##_handle, name& > result = {                                 \
      name##_handle( handle_index, entry->counter ),                           \
      m_##name##s[ m_##name##s_size ] };                                       \
  /* update this */                                                            \
  m_##name##s_next_free_handle_slot = entry->next_free_index;                  \
  ++m_##name##s_size;                                                          \
  return result;

    std::pair< atom_handle, atom& > create_atom()
    {
      CREATE_ELEMENT( atom )
    }

    std::pair< link_handle, link& > create_link()
    {
      CREATE_ELEMENT( link )
    }

    std::pair< face_handle, face& > create_face()
    {
      CREATE_ELEMENT( face )
    }

# undef CREATE_ELEMENT

# define GROW_ELEMENTS( name )                                                 \
  if(  new_capacity <= m_##name##s_capacity                                    \
    || new_capacity > max_##name##_handle_index )                              \
    throw skeleton_##name##_buffer_overflow( __FILE__, __LINE__ );             \
  if( !m_##name##s )                                                           \
    {                                                                          \
      m_##name##s = new name[ new_capacity ];                                  \
      m_##name##_index_to_handle_index = new size_t[ new_capacity ];           \
      m_##name##_handles = new name##_handle_entry[ new_capacity ];            \
    }                                                                          \
  else                                                                         \
    {                                                                          \
      auto new_element_buffer = new name[ new_capacity ];                      \
      auto new_element_to_handle = new size_t[ new_capacity ];                 \
      auto new_handle_buffer = new name##_handle_entry[ new_capacity ];        \
      _Pragma("omp parallel for")                                              \
      for( size_t i = 0; i < m_##name##s_capacity; ++ i )                      \
        {                                                                      \
          new_element_buffer[ i ] = std::move( m_##name##s[ i ] );             \
          new_element_to_handle[ i ] = m_##name##_index_to_handle_index[ i ];  \
          new_handle_buffer[ i ] = m_##name##_handles[ i ];                    \
        }                                                                      \
      delete[] m_##name##s;                                                    \
      delete[] m_##name##_index_to_handle_index;                               \
      delete[] m_##name##_handles;                                             \
                                                                               \
      m_##name##s = new_element_buffer;                                        \
      m_##name##_index_to_handle_index = new_element_to_handle;                \
      m_##name##_handles = new_handle_buffer;                                  \
    }                                                                          \
   _Pragma("omp parallel for")                                                 \
   for( size_t i = m_##name##s_capacity; i < new_capacity; ++ i )              \
     m_##name##_handles[ i ].next_free_index = i + 1;                          \
   m_##name##s_capacity = new_capacity;

    void grow_atoms( size_t new_capacity )
    {
      GROW_ELEMENTS( atom )
    }
    void grow_links( size_t new_capacity )
    {
      GROW_ELEMENTS( link )
    }
    void grow_faces( size_t new_capacity )
    {
      GROW_ELEMENTS( face )
    }
# undef GROW_ELEMENTS

    void remove( atom_handle h )
    {
      if( h.index >= m_atoms_capacity )
        MP_THROW_EXCEPTION( skeleton_invalid_atom_handle );
        throw skeleton_invalid_atom_handle( __FILE__, __LINE__ );
      auto entry = m_atom_handles + h.index;
    }

    atom* m_atoms;
    size_t* m_atom_index_to_handle_index;
    atom_handle_entry* m_atom_handles;

    link* m_links;
    size_t* m_link_index_to_handle_index;
    link_handle_entry* m_link_handles;

    face* m_faces;
    size_t* m_face_index_to_handle_index;
    face_handle_entry* m_face_handles;

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
    face_handle_type, face_handle_index_bits>::face_handle_bits;

  template<
    typename atom_handle_type, uint8_t atom_handle_index_bits,
    typename link_handle_type, uint8_t link_handle_index_bits,
    typename face_handle_type, uint8_t face_handle_index_bits >
  constexpr size_t skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::max_atom_handle_index;

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  constexpr size_t skeleton_datastructure<
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
  constexpr size_t skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::max_link_handle_index;

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  constexpr size_t skeleton_datastructure<
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
  constexpr size_t skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::max_face_handle_index;

  template<
      typename atom_handle_type, uint8_t atom_handle_index_bits,
      typename link_handle_type, uint8_t link_handle_index_bits,
      typename face_handle_type, uint8_t face_handle_index_bits >
  constexpr size_t skeleton_datastructure<
    atom_handle_type, atom_handle_index_bits,
    link_handle_type, link_handle_index_bits,
    face_handle_type, face_handle_index_bits>::max_face_handle_counter;

END_MP_NAMESPACE
# endif 
