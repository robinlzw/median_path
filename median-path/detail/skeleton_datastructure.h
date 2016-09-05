# ifndef MEDIAN_PATH_SKELETON_DATASTRUCTURE_H_
# define MEDIAN_PATH_SKELETON_DATASTRUCTURE_H_

# include "../median_path.h"
# include "exceptions.h"

# include <graphics-origin/geometry/vec.h>
# include <graphics-origin/geometry/ball.h>

# include <type_traits>
# include <vector>

# ifdef MP_SKELETON_NO_CHECK
# define MP_SKELETON_NO_INDEX_CHECK
# define MP_SKELETON_NO_HANDLE_CHECK
# define MP_SKELETON_NO_POINTER_CHECK
# endif

namespace median_path {
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
   * packed in a buffer. Data can be added to a category dynamically, but there
   * are some data that will always be stored in a skeleton:
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
    static constexpr atom_handle_type max_atom_handle_index = ( atom_handle_type{1} << atom_handle_index_bits ) - 1;
    static constexpr atom_handle_type max_atom_handle_counter = ( atom_handle_type{1} << (atom_handle_bits - atom_handle_index_bits - 2 )) - 1;

    static constexpr uint8_t link_handle_bits = sizeof( link_handle_type ) << 3;
    static constexpr link_handle_type max_link_handle_index = ( link_handle_type{1} << link_handle_index_bits ) - 1;
    static constexpr link_handle_type max_link_handle_counter = ( link_handle_type{1} << (link_handle_bits - link_handle_index_bits - 2 )) - 1;

    static constexpr uint8_t face_handle_bits = sizeof( face_handle_type ) << 3;
    static constexpr face_handle_type max_face_handle_index = ( face_handle_type{1} << face_handle_index_bits ) - 1;
    static constexpr face_handle_type max_face_handle_counter = ( face_handle_type{1} << (face_handle_bits - face_handle_index_bits - 2 )) - 1;

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
      atom_handle_entry();
    };
    struct link_handle_entry {
      link_handle_type next_free_index : link_handle_index_bits;
      link_handle_type counter         : link_handle_bits - link_handle_index_bits - 2;
      link_handle_type status          : 2;
      link_handle_type link_index;
      link_handle_entry();
    };
    struct face_handle_entry {
      face_handle_type next_free_index : face_handle_index_bits;
      face_handle_type counter         : face_handle_bits - face_handle_index_bits - 2;
      face_handle_type status          : 2;
      face_handle_type face_index;
      face_handle_entry();
    };

    /**************************************************************************
     * HANDLES:                                                               *
     *  - identifiers of an element                                           *
     **************************************************************************/
    struct atom_handle {
      atom_handle_type index  : atom_handle_index_bits;
      atom_handle_type counter: atom_handle_bits - atom_handle_index_bits;
      atom_handle();
      atom_handle( atom_handle_type idx, atom_handle_type ctr );
      inline operator atom_handle_type() const noexcept;
      inline bool is_valid() const noexcept;
    };
    struct link_handle {
      link_handle_type index  : link_handle_index_bits;
      link_handle_type counter: link_handle_bits - link_handle_index_bits;
      link_handle();
      link_handle( link_handle_type idx, link_handle_type ctr );
      inline operator link_handle_type() const noexcept;
      inline bool is_valid() const noexcept;
    };
    struct face_handle {
      face_handle_type index  : face_handle_index_bits;
      face_handle_type counter: face_handle_bits - face_handle_index_bits;
      face_handle();
      face_handle( face_handle_type idx, face_handle_type ctr );
      inline operator face_handle_type() const noexcept;
      bool is_valid() const noexcept;
    };

    /**************************************************************************
     * INDICES:                                                               *
     * Even if an index type is just an alias on the corresponding handle,    *
     * having a specific type name improve the readability and the semantic.  *
     **************************************************************************/
    typedef atom_handle_type atom_index;
    typedef link_handle_type link_index;
    typedef face_handle_type face_index;

    /**************************************************************************
     * ELEMENT TYPES:                                                         *
     *  - an atom is a ball                                                   *
     *  - a link is a pair of atom handles                                    *
     *  - a face is a 3-cycle of links along with their atom handles          *
     **************************************************************************/
    typedef GO_NAMESPACE::geometry::ball atom;
    struct link {
      link();
      link& operator=( link&& other );
      atom_handle h1;
      atom_handle h2;
    };
    struct face {
      face();
      face& operator=( face&& other );
      atom_handle atoms[ 3 ];
      link_handle links[ 3 ];
    };

    /**************************************************************************
     * ELEMENT DATA TYPES:                                                    *
     *  - a common structure to be stored in a container                      *
     *  - five virtual functions to be able to call the constructor and       *
     *  destructors of the actual types:                                      *
     *     a) the property buffer destructor                                  *
     *     b) the property buffer resize                                      *
     *     c) move an element inside the property buffer                      *
     *     d) destroy a particular element in the buffer                      *
     *     e) destroy a range of particular elements in the buffer            *
     **************************************************************************/
    struct base_property_buffer {
      base_property_buffer( size_t size, const std::string& name );

      virtual ~base_property_buffer();

      template< typename T >
      inline T& get( size_t index );

      void clear( size_t current_capacity );

      virtual void resize( size_t old_capacity, size_t new_capacity ) = 0;

      virtual void move( size_t from, size_t to ) = 0;

      virtual void destroy( size_t index ) = 0;

      virtual void destroy( size_t from, size_t end ) = 0;

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


      derived_property_buffer( const std::string& name );
      void destroy( size_t index ) override;
      void destroy( size_t from, size_t end ) override;
      void resize( size_t old_capacity, size_t new_capacity ) override;
      void move( size_t from, size_t to ) override;
      ~derived_property_buffer();
    };

    /**************************************************************************
     * CONSTRUCTION / COPY / DESTRUCTION:                                     *
     * manage the life cycle of a skeleton data structure.                    *
     **************************************************************************/
    /** Create an empty but valid skeleton data structure.*/
    skeleton_datastructure();
    /** Create a skeleton data structure with enough space allocated to hold the
     * specified number of elements. This constructor can throw an exception
     * if one of the requested capacity is above the representation or the
     * memory limits.
     * @param nb_atoms Maximum number of atoms this data structure can store
     * without any reallocation.
     * @param nb_links Maximum number of links this data structure can store
     * without any reallocation.
     * @param nb_faces Maximum number of faces this data structure can store
     * without any reallocation. */
    skeleton_datastructure( atom_handle_type nb_atoms, link_handle_type nb_links = 0, face_handle_type nb_faces = 0 );
    ~skeleton_datastructure();
    skeleton_datastructure( skeleton_datastructure& other ) = delete;
    skeleton_datastructure( skeleton_datastructure&& other ) = delete;
    skeleton_datastructure& operator=( skeleton_datastructure&& other ) = delete;
    skeleton_datastructure& operator=( const skeleton_datastructure& other ) = delete;

    /**************************************************************************
     * HELPER FUNCTIONS:                                                      *
     * resize / reset buffers. Note that those functions can throw. None of   *
     * those methods is thread safe.                                          *
     **************************************************************************/
    void clear( atom_handle_type atom_capacity, link_handle_type link_capacity, face_handle_type face_capacity );
    void clear_atoms( atom_handle_type atom_capacity );
    void clear_links( link_handle_type link_capacity );
    void clear_faces( face_handle_type face_capacity );

    void grow_atoms( atom_handle_type new_capacity );
    void grow_links( link_handle_type new_capacity );
    void grow_faces( face_handle_type new_capacity );

    /**************************************************************************
     * ELEMENT CREATION & REMOVAL:                                            *
     * create new elements or delete existing ones. All creation methods may  *
     * throw. Deletion methods could throw if index/handle/pointer checks are *
     * activated. None of this methods is thread safe.                        *
     **************************************************************************/
    /**Create a new atom.
     * @return A pair with the handle of the new atom and a reference to it.
     */
    std::pair< atom_handle, atom& > create_atom();
    /**Create a new link.
     * @return A pair with the handle of the new link and a reference to it.
     */
    std::pair< link_handle, link& > create_link();
    /**Create a new face.
     * @return A pair with the handle of the new face and a reference to it.
     */
    std::pair< face_handle, face& > create_face();

    /**For those three methods, if the handle is valid, the corresponding
     * element is removed. When the handle is invalid, an exception is
     * thrown. */
    void remove( atom_handle h );
    void remove( link_handle h );
    void remove( face_handle h );

    /**Those three methods remove an element given a pointer to this
     * element. If such pointer is invalid, an exception is thrown.*/
    void remove( atom& e );
    void remove( link& e );
    void remove( face& e );

    /**Those three methods
     *
     * @note The name is not consistent with other remove methods to
     * avoid confusion between atom handle
     */
    void remove_atom_by_index( atom_index index );
    void remove_link_by_index( link_index index );
    void remove_face_by_index( face_index index );

    /**************************************************************************
     * ACCESS ELEMENTS, HANDLES AND INDICES:                                  *
     * access to elements, get their handles and their indices. If index,     *
     * handle or pointer checks are activated, exception can be thrown. Those *
     * methods are thread safe.                                               *
     **************************************************************************/
    atom& get( atom_handle h );
    link& get( link_handle h );
    face& get( face_handle h );

    atom& get_atom_by_index( atom_index index );
    link& get_link_by_index( link_index index );
    face& get_face_by_index( face_index index );

    atom_handle get_atom_handle( atom_index index );
    link_handle get_link_handle( link_index index );
    face_handle get_face_handle( face_index index );

    atom_index get_index( atom_handle handle );
    link_index get_index( link_handle handle );
    face_index get_index( face_handle handle );

    atom_index get_index( atom& e );
    link_index get_index( link& e );
    face_index get_index( face& e );

    atom_handle get_handle( atom& e );
    link_handle get_handle( link& e );
    face_handle get_handle( face& e );

    template< typename T >
    void add_atom_property( const std::string& name );

    template< typename T >
    void add_link_property( const std::string& name );

    template< typename T >
    void add_face_property( const std::string& name );

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

# define dts_template_parameters                               \
 template<                                                     \
   typename atom_handle_type, uint8_t atom_handle_index_bits,  \
   typename link_handle_type, uint8_t link_handle_index_bits,  \
   typename face_handle_type, uint8_t face_handle_index_bits >

# define dts_type                                              \
  skeleton_datastructure<                                      \
  atom_handle_type, atom_handle_index_bits,                    \
  link_handle_type, link_handle_index_bits,                    \
  face_handle_type, face_handle_index_bits>

# define dts_definition(ret_type)                              \
  dts_template_parameters                                      \
  ret_type dts_type

  dts_definition(constexpr uint8_t)::atom_handle_bits;
  dts_definition(constexpr atom_handle_type)::max_atom_handle_index;
  dts_definition(constexpr atom_handle_type)::max_atom_handle_counter;

  dts_definition(constexpr uint8_t)::link_handle_bits;
  dts_definition(constexpr link_handle_type)::max_link_handle_index;
  dts_definition(constexpr link_handle_type)::max_link_handle_counter;

  dts_definition(constexpr uint8_t)::face_handle_bits;
  dts_definition(constexpr face_handle_type)::max_face_handle_index;
  dts_definition(constexpr face_handle_type)::max_face_handle_counter;
# include "skeleton_datastructure_handle_and_entries.tcc"
# include "skeleton_datastructure_elements_and_properties.tcc"
# include "skeleton_datastructure_construction_destruction.tcc"
# include "skeleton_datastructure_helper.tcc"
# include "skeleton_datastructure_element_creation_and_deletion.tcc"
# include "skeleton_datastructure_elements_access.tcc"
# include "skeleton_datastructure_property_management.tcc"
# undef dts_template_parameters
# undef dts_type
# undef dts_definition
} //median_path
# endif 
