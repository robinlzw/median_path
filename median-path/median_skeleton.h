/* Created on: Mar 1, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# ifndef MEDIAN_PATH_MEDIAN_SKELETON_H_
# define MEDIAN_PATH_MEDIAN_SKELETON_H_

# include "detail/skeleton_datastructure.h"
# include <graphics-origin/geometry/vec.h>

# include <string>
# include <stdint.h>
BEGIN_MP_NAMESPACE

  using graphics_origin::vec3;
  using graphics_origin::vec4;

  /**@brief A median skeleton class.
   *
   * A skeleton is a thin structure, centered in a shape, that jointly describes
   * the topology and the geometry of such shape. It is composed of atoms, which
   * in the case of a median skeleton, are maximum inscribed balls. Those balls
   * are represented here by a tight buffer of vec4: xyz for the atom position
   * (aka center or skeleton point or medial point) and w for the atom radius.
   * The topology of the shape is captured by the atom neighborhoods and explicitly
   * stored in links and faces connecting atoms. Atoms, links and faces are the
   * skeleton elements.
   *
   * For most methods, you have the choice to identify skeleton elements by
   * handles, references or indices. Handles are safer to identify elements,
   * but also slower. References and indices are faster, but could throw
   * exception if they are incorrect. Indeed, both references and indices are
   * subject to change. Thus, you might have references or indices that were
   * correct at the moment you obtained them, but operations that happened
   * after invalidated them. Adding and removing elements are the operations that
   * invalidate references and indices. The handle of an element that was not
   * deleted is always valid.
   *
   * By default, all handles, references and indices are checked. If one of
   * them is invalid, an exception is thrown:
   * - skeleton_invalid_atom_index, skeleton_invalid_link_index, or
   * skeleton_invalid_face_index for invalid indices,
   * - skeleton_invalid_atom_handle, skeleton_invalid_link_handle or
   * skeleton_invalid_face_handle for invalid handles,
   * - skeleton_invalid_atom_pointer, skeleton_invalid_link_pointer or
   * skeleton_invalid_face_pointer for invalid pointers.
   * You can deactivate those checks by defining the following symbols before
   * including medial_skeleton.h:
   * - MP_SKELETON_NO_INDEX_CHECK
   * - MP_SKELETON_NO_HANDLE_CHECK
   * - MP_SKELETON_NO_POINTER_CHECK
   * - MP_SKELETON_NO_CHECK (to define the three previous symbols).
   *
   * For each skeleton elements, you have methods to:
   * - add a new element
   * - remove an existing element
   * - get an existing element, or its handle or its index
   * - process all elements
   * - filter elements (select which elements will be removed).
   *
   * Maximum capacities:
   *   - 2^22 atoms
   *   - 2^44 links
   *   - 2^54 faces
   */
  class median_skeleton
  {
    typedef skeleton_datastructure< uint32_t, 22, uint64_t, 44, uint64_t, 54 > datastructure;
  public:
    ///
    typedef datastructure::atom atom;
    typedef datastructure::link link;
    typedef datastructure::face face;

    typedef datastructure::atom_handle atom_handle;
    typedef datastructure::link_handle link_handle;
    typedef datastructure::face_handle face_handle;

    typedef uint32_t atom_index;
    typedef uint64_t link_index;
    typedef uint64_t face_index;

    /**@brief Map an atom to a face passing by this atom.
     *
     * Let us suppose that a face with handle f connects three links and three atoms
     * in the following order: a1 --(l1)--> a2 --(l2)--> a3 --(l3)--> a1. This
     * structure establish the map of the atom a1 to the face f:
     * links = {l1,l2,l3}
     * atoms = {a2,a3}
     * face  = f
     */
    struct atom_face_element
    {
      link_handle links[3];
      atom_handle atoms[2];
      face_handle face;

      atom_face_element(
        datastructure::face& f, face_handle fh, ushort i );
      atom_face_element( );
      atom_face_element(
        atom_face_element&& other );
      atom_face_element&
      operator=(
        atom_face_element&& other );
    };

    /**@brief Map a link to a face passing by this link.
     *
     * Let us suppose that a face with handle f connects three links and three atoms
     * in the following order: a1 --(l1)--> a2 --(l2)--> a3 --(l3)--> a1. This
     * structure establish the map of the atom l1 to the face f:
     * links = {l2,l3}
     * opposite = a3
     * face  = f
     */
    struct link_face_element
    {
      link_handle links[2];
      atom_handle opposite;
      face_handle face;

      link_face_element(
        datastructure::face& f, face_handle fh, ushort i );
      link_face_element( );
      link_face_element(
        link_face_element&& other );
      link_face_element&
      operator=(
        link_face_element&& other );
    };

    typedef std::vector< std::pair< link_handle, atom_handle > > atom_links_property;
    typedef std::vector< atom_face_element > atom_faces_property;
    typedef std::vector< link_face_element > link_faces_property;

    /**@name Construction & Destruction
     * @{ */
    /**@brief Build a median skeleton.
     *
     * Build a median skeleton with enough place to store the requested
     * number of atoms, links and faces.
     * @param atom_capacity Requested atom capacity
     * @param link_capacity Requested link capacity
     * @param face_capacity Requested face capacity
     */
    median_skeleton(
      atom_index atom_capacity = 0, uint64_t link_capacity = 0,
      uint64_t face_capacity = 0 );
    /**@brief Copy constructor
     *
     * Copy a skeleton into this. This operation is very fast as it is merely
     * a swap between two pointers. At the end of the copy, the other skeleton
     * is in a undefined, but valid, state. */
    median_skeleton(
      median_skeleton&& other );
    /**@brief Build from file
     *
     * Build a median skeleton from a file.
     * @param filename The path of the file describing the skeleton to build. */
    median_skeleton(
      const std::string& filename );

    median_skeleton&
    operator=(
      median_skeleton&& other );

    ~median_skeleton( );
    ///@}

    /**@name Utilities
     * @{ */
    /**@brief Clear the internal structures of the skeleton.
     *
     * After the call, the skeleton is empty but with enough space to store the
     * requested number of atom, link and faces. The previous atom, link and
     * face properties still exist in the skeleton but are now empty. If a requested
     * capacity is smaller than the current capacity, the corresponding buffer is not
     * shrinked.
     * @param atom_capacity Requested atom capacity
     * @param link_capacity Requested link capacity
     * @param face_capacity Requested face capacity */
    void
    clear(
      atom_index atom_capacity = 0, link_index link_capacity = 0,
      face_index face_capacity = 0 );

    /**@brief Reserve place for atoms.
     *
     * Expand atom buffers to the requested capacity. If the requested capacity is smaller
     * than the current one, nothing is done since it is not clear which atoms should be
     * destroyed. If the requested capacity is bigger than the current one, all buffers
     * are reallocated.
     * @param new_atoms_capacity The requested new atom*/
    void
    reserve_atoms(
      atom_index new_atoms_capacity );

    /**@brief Reserve place for links.
     *
     * Expand link buffers to the requested capacity. If the requested capacity is smaller
     * than the current one, nothing is done since it is not clear which links should be
     * destroyed. If the requested capacity is bigger than the current one, all buffers
     * are reallocated.
     * @param new_links_capacity The requested new link*/
    void
    reserve_links(
      link_index new_links_capacity );

    /**@brief Reserve place for faces.
     *
     * Expand face buffers to the requested capacity. If the requested capacity is smaller
     * than the current one, nothing is done since it is not clear which faces should be
     * destroyed. If the requested capacity is bigger than the current one, all buffers
     * are reallocated.
     * @param new_faces_capacity The requested new faces capacity.*/
    void
    reserve_faces(
      face_index new_faces_capacity );

    /**@brief Load a skeleton from a file.
     *
     * Load a median skeleton described by a file into this skeleton.
     * @param filename The path of the file describing the skeleton to build.
     * @return true if the load was successful */
    bool
    load(
      const std::string& filename );

    /**@brief Save a skeleton to a file.
     *
     * Save this skeleton to a file.
     * @param filename The path of the file to write this skeleton into.
     * @return true if the save was successful */
    bool
    save(
      const std::string& filename );

    /**@brief Get the current number of atoms
     *
     * Get the number of atoms currently in this skeleton.
     * @return The current number of atoms. */
    atom_index
    get_number_of_atoms( ) const noexcept;
    /**@brief Get the current number of links
     *
     * Get the number of links currently in this skeleton.
     * @return The current number of links. */
    link_index
    get_number_of_links( ) const noexcept;
    /**@brief Get the current number of faces
     *
     * Get the number of faces currently in this skeleton.
     * @return The current number of faces. */
    face_index
    get_number_of_faces( ) const noexcept;

    /**@brief Get the atoms capacity.
     *
     * Get the maximum number of atoms that can be handled by this skeleton
     * without resizing its internal buffers.
     * @return The atoms capacity. */
    atom_index
    get_atoms_capacity( ) const noexcept;
    /**@brief Get the links capacity.
     *
     * Get the maximum number of links that can be handled by this skeleton
     * without resizing its internal buffers.
     * @return The links capacity. */
    link_index
    get_links_capacity( ) const noexcept;
    /**@brief Get the faces capacity.
     *
     * Get the maximum number of faces that can be handled by this skeleton
     * without resizing its internal buffers.
     * @return The faces capacity. */
    face_index
    get_faces_capacity( ) const noexcept;

    /**@brief Get the number of user atom properties.
     *
     * Get the number of properties associated to atoms. This number does not take
     * into account the internal atom properties used to manage the skeleton topology.
     * @return The number of user atom properties.
     */
    uint64_t
    get_number_of_atom_properties( ) const noexcept;

    /**@brief Get the number of user link properties.
     *
     * Get the number of properties associated to links. This number does not take
     * into account the internal link properties used to manage the skeleton topology.
     * @return The number of user link properties.
     */
    uint64_t
    get_number_of_link_properties( ) const noexcept;

    /**@brief Get the number of user face properties.
     *
     * Get the number of properties associated to faces. This number does not take
     * into account the internal face properties used to manage the skeleton topology.
     * @return The number of user face properties.
     */
    uint64_t
    get_number_of_face_properties( ) const noexcept;

    /**@brief Compute the bounding box of atoms.
     *
     * This function computes the bounding box of all atoms contained in
     * this skeleton.
     * @param b The resulting bounding box. */
    void
    compute_bounding_box(
      graphics_origin::geometry::aabox& b ) const;

    /**@brief Compute the bounding box of atom centers.
     *
     * This function computes the bounding box of all atom centers in this
     * skeleton.
     * @param b The resulting bounding box.
     */
    void
    compute_centers_bounding_box(
      graphics_origin::geometry::aabox& b ) const;

    /**@brief Compute min and max radii.
     *
     * This function computes the minimum and maximum radii of atoms in this
     * skeleton.
     * @param minr The minimum radius.
     * @param maxr The maximum radius.*/
    void
    compute_minmax_radii(
      real& minr, real& maxr ) const;

    /**@name Atom management
     * @{ */
    /**@brief Add an atom to the skeleton.
     *
     * Add an atom to this skeleton with the center and the radius
     * sent separately.
     * @param position Center of the atom to add.
     * @param radius Radius of the atom to add.
     * @return An atom handle that points to the newly created atom. */
    atom_handle
    add(
      const vec3& position, const real& radius );
    /**@brief Add an atom to the skeleton.
     *
     * Add an atom to this skeleton with its geometry stored in a
     * vec4.
     * @param ball A vec4 with xyz describing the atom center and w
     * storing the radius.
     * @return An atom handle that points to the newly created atom. */
    atom_handle
    add(
      const vec4& ball );

    /**@brief Remove an atom know by its handle.
     *
     * Remove an atom from the skeleton. Its faces and links will
     * be destroyed too. If the handle is invalid (e.g. the atom
     * is already removed), the error is silently ignored.
     * @param handle Handle of the atom to remove.
     */
    void
    remove(
      atom_handle handle );

    /**@brief Remove an atom known by a reference.
     *
     * Remove an atom from the skeleton. Its faces and links will
     * be destroyed too. If the atom points to memory that is not
     * tagged as a valid atom, an exception is thrown.
     * @param e Reference to the atom to remove.
     */
    void
    remove(
      atom& e );

    /**@brief Access to an atom known by an handle.
     *
     * Access to an atom thank to its handle. If the handle is
     * incorrect, an exception is thrown. Be careful to not store this
     * reference if you plan to add/remove atoms after, since it will
     * invalidate the reference.
     * @param handle Handle of the atom in the tight buffer.
     */
    atom&
    get(
      atom_handle handle ) const;

    /**@brief Access to an atom known by an index.
     *
     * Access to an atom thank to its index. If the index is
     * incorrect, an exception is thrown. Be careful to not store this
     * reference if you plan to add/remove atoms after, since it will
     * invalidate the reference.
     * @param index Index of the atom in the tight buffer.
     */
    atom&
    get_atom_by_index(
      atom_index index ) const;

    /**@brief Get the index of an atom known by its handle.
     *
     * Get the index of an atom known by an handle. If the handle does
     * not point to a valid atom, an exception is thrown.
     * @param handle Handle of the atom we want the index.
     */
    atom_index
    get_index(
      atom_handle handle ) const;
    /**@brief Get the index of an atom known by a reference.
     *
     * Get the index of an atom known by a reference. If the reference does
     * not point to a valid atom, an exception is thrown.
     * @param e Reference of the atom we want the index.
     */
    atom_index
    get_index(
      atom& e ) const;
    /**@brief Get the handle of an atom.
     *
     * Get the handle of an atom. If the atom points to memory that is
     * not tagged as a valid atom, an exception is thrown.
     * @param e Reference to the atom we want to know the handle.
     */
    atom_handle
    get_handle(
      atom& e ) const;
    /**@brief Check if an atom handle is valid.
     *
     * An atom handle is valid if it identifies a currently allocated
     * atom in the tight buffer. This function check if an handle is
     * valid.
     * @param handle The handle to check.
     */
    bool
    is_valid(
      atom_handle handle ) const;

    /**@brief Get the number of links of an atom.
     *
     * Get the number of links attached to an atom known by reference. If
     * the reference does not point to a valid atom, an exception is thrown.
     * @param e Reference the atom.
     * @return The number of links */
    link_index
    get_number_of_links(
      atom& e ) const;
    /**@brief Get the number of links of an atom.
     *
     * Get the number of links attached to an atom known by its index. If
     * the index is invalid, an exception is thrown.
     * @param index Index of the atom.
     * @return The number of links */
    link_index
    get_number_of_links(
      atom_index index ) const;
    /**@brief Get the number of links of an atom.
     *
     * Get the number of links attached to an atom known by its handle. If
     * the handle is invalid, an exception is thrown.
     * @param handle Handle of the atom.
     * @return The number of links */
    link_index
    get_number_of_links(
      atom_handle h ) const;

    /**@brief Apply a process function on all atoms.
     *
     * This method apply, in parallel or not, a function on each
     * valid atom.
     * @param function The function to apply.
     * @param parallel A flag to activate a parallel processing.
     */
    template< typename atom_processer >
      void
      process_atoms(
        atom_processer&& function, bool parallel = true );

    /**@brief Filter atoms according to a filter function.
     *
     * This method select atoms to remove thanks to a filter function. The
     * evaluation of the filter function can be done in parallel. This method
     * guarantees that the atoms are still stored in a tight buffer after
     * removals. This function will invalidate references and indices to atoms.
     * @param filter The filter function used to select atoms to remove.
     * @param parallel A flag to activate a parallel evaluation of the filter function.
     */
    template< typename atom_filter >
      void
      remove_atoms(
        atom_filter&& filter, bool parallel = true );

    /**@name Link management
     * @{ */
    /**@brief Add a link to the skeleton.
     *
     * Add a link between two atoms. If one of the handles is invalid,
     * an exception is thrown. If the link already exists, its handle is returned.
     * @param handle1 The handle of the first atom.
     * @param handle2 The handle of the second atom.
     * @return An link handle that points to the newly created link. */
    link_handle
    add(
      atom_handle handle1, atom_handle handle2 );
    /**@brief Add a link to the skeleton.
     *
     * Add a link between two atoms. If one of the atom references is invalid,
     * an exception is thrown. If the link already exists, its handle is returned.
     * @param atom1 Reference to the first atom.
     * @param atom2 Reference to the second atom.
     * @return An link handle that points to the newly created link. */
    link_handle
    add(
      atom& atom1, atom& atom2 );
    /**@brief Add a link to the skeleton.
     *
     * Add a link between two atoms. If one of the atom indices is invalid,
     * an exception is thrown. If the link already exists, its handle is
     * returned.
     * @param idx1 Index of the first atom.
     * @param idx2 Index of the second atom.
     * @return An link handle that points to the newly created link. */
    link_handle
    add(
      atom_index idx1, atom_index idx2 );
    /**@brief Remove a link know by its handle.
     *
     * Remove a link from the skeleton. Its faces will
     * be destroyed too. Also, its atoms will be notified that they are
     * no longer connected by the corresponding link and faces.
     * If the handle is invalid (e.g. the link
     * is already removed), the error is silently ignored.
     * @param handle Handle of the link to remove.
     */
    void
    remove(
      link_handle handle );

    /**@brief Remove a link known by a reference.
     *
     * Remove a link from the skeleton. Its faces will
     * be destroyed too. Also, its atoms will be notified that they are
     * no longer connected by the corresponding link and faces.
     * If the link points to memory that is not
     * tagged as a valid link, an exception is thrown.
     * @param e Reference to the link to remove.
     */
    void
    remove(
      link& e );

    /**@brief Access to a link known by an handle.
     *
     * Access to a link thank to its handle. If the handle is
     * incorrect, an exception is thrown. Be careful to not store this
     * reference if you plan to add/remove links after, since it will
     * invalidate the reference.
     * @param handle Handle of the link in the tight buffer.
     */
    link&
    get(
      link_handle handle ) const;

    /**@brief Access to a link known by an index.
     *
     * Access to a link thank to its index. If the index is
     * incorrect, an exception is thrown. Be careful to not store this
     * reference if you plan to add/remove links after, since it will
     * invalidate the reference.
     * @param index Index of the link in the tight buffer.
     */
    link&
    get_link_by_index(
      link_index index ) const;

    /**@brief Get the index of a link known by its handle.
     *
     * Get the index of a link known by an handle. If the handle does
     * not point to a valid link, an exception is thrown.
     * @param handle Handle of the link we want the index.
     */
    link_index
    get_index(
      link_handle handle ) const;
    /**@brief Get the index of a link known by a reference.
     *
     * Get the index of a link known by a reference. If the reference does
     * not point to a valid link, an exception is thrown.
     * @param e Reference of the link we want the index.
     */
    link_index
    get_index(
      link& e ) const;
    /**@brief Get the handle of a link.
     *
     * Get the handle of a link. If the link points to memory that is
     * not tagged as a valid link, an exception is thrown.
     * @param e Reference to the link we want to know the handle.
     */
    link_handle
    get_handle(
      link& e ) const;
    /**@brief Check if a link handle is valid.
     *
     * An link handle is valid if it identifies a currently allocated
     * link in the tight buffer. This function check if an handle is
     * valid.
     * @param handle The handle to check.
     */
    bool
    is_valid(
      link_handle handle ) const;

    /**@brief Check if two atoms are connected by a link.
     *
     * Check if a link exist between two atoms.
     * @param idx1 index of the first atom
     * @param idx2 index of the second atom
     * @return True if the two atoms are connected.*/
    bool is_a_link(
      atom_index idx1,
      atom_index idx2 ) const;

    /**@brief Apply a process function on all links.
     *
     * This method apply, in parallel or not, a function on each
     * valid link.
     * @param function The function to apply.
     * @param parallel A flag to activate a parallel processing.
     */
    template< typename link_processer >
      void
      process_links(
        link_processer&& function, bool parallel = true );

    /**@brief Filter links according to a filter function.
     *
     * This method select links to remove thanks to a filter function. The
     * evaluation of the filter function can be done in parallel. This method
     * guarantees that the links are still stored in a tight buffer after
     * removals. This function will invalidate references and indices to links.
     * @param filter The filter function used to select links to remove.
     * @param parallel A flag to activate a parallel evaluation of the filter function. */
    template< typename link_filter >
      void
      remove_links(
        link_filter&& filter, bool parallel = true );

    /**@brief Get the number of faces of an link.
     *
     * Get the number of faces attached to an link known by reference. If
     * the reference does not point to a valid link, an exception is thrown.
     * @param e Reference the link.
     * @return The number of faces */
    face_index
    get_number_of_faces(
      link& e ) const;
    /**@brief Get the number of faces of an link.
     *
     * Get the number of faces attached to an link known by its index. If
     * the index is invalid, an exception is thrown.
     * @param index Index of the link.
     * @return The number of faces */
    face_index
    get_number_of_faces(
      link_index index ) const;
    /**@brief Get the number of faces of an link.
     *
     * Get the number of faces attached to an link known by its hande. If
     * the handle is invalid, an exception is thrown.
     * @param handle Handle of the link.
     * @return The number of faces */
    face_index
    get_number_of_faces(
      link_handle h ) const;

    /**@name Face management
     * @{ */
    /**@brief Add a face to the skeleton.
     *
     * Add a face between three atoms. If one of the handles is invalid,
     * an exception is thrown. If the face already exists, its handle is returned.
     * The links between the three atoms are added if they do not exist.
     * @param handle1 The handle of the first atom.
     * @param handle2 The handle of the second atom.
     * @param handle3 The handle of the third atom.
     * @return An face handle that points to the newly created face. */
    face_handle
    add(
      atom_handle handle1, atom_handle handle2, atom_handle handle3 );
    /**@brief Add a face to the skeleton.
     *
     * Add a face between three atoms. If one of the atom references is invalid,
     * an exception is thrown. If the face already exists, its handle is returned.
     * The links between the three atoms are added if they do not exist.
     * @param atom1 Reference to the first atom.
     * @param atom2 Reference to the second atom.
     * @param atom3 Reference to the third atom.
     * @return An face handle that points to the newly created face. */
    face_handle
    add(
      atom& atom1, atom& atom2, atom& atom3 );
    /**@brief Add a face to the skeleton.
     *
     * Add a face between three atoms. If one of the atom indices is invalid,
     * an exception is thrown. If the face already exists, its handle is
     * returned.
     * The links between the three atoms are added if they do not exist.
     * @param idx1 Index of the first atom.
     * @param idx2 Index of the second atom.
     * @param idx3 Index of the third atom.
     * @return An face handle that points to the newly created face. */
    face_handle
    add(
      atom_index idx1, atom_index idx2, atom_index idx3 );
    /**@brief Remove a face know by its handle.
     *
     * Remove a face from the skeleton. Its atoms and links will be
     * notified that they are no longer connected by the corresponding face.
     * If the handle is invalid (e.g. the face is already removed), the error
     * is silently ignored.
     * @param handle Handle of the face to remove.
     */
    void
    remove(
      face_handle handle );

    /**@brief Remove a face known by a reference.
     *
     * Remove a face from the skeleton. Its atoms and links will be
     * notified that they are no longer connected by the corresponding face.
     * If the face points to memory that is not
     * tagged as a valid face, an exception is thrown.
     * @param e Reference to the face to remove.
     */
    void
    remove(
      face& e );

    /**@brief Access to a face known by an handle.
     *
     * Access to a face thank to its handle. If the handle is
     * incorrect, an exception is thrown. Be careful to not store this
     * reference if you plan to add/remove faces after, since it will
     * invalidate the reference.
     * @param handle Handle of the face in the tight buffer.
     */
    face&
    get(
      face_handle handle ) const;

    /**@brief Access to a face known by an index.
     *
     * Access to a face thank to its index. If the index is
     * incorrect, an exception is thrown. Be careful to not store this
     * reference if you plan to add/remove faces after, since it will
     * invalidate the reference.
     * @param index Index of the face in the tight buffer.
     */
    face&
    get_face_by_index(
      face_index index ) const;

    /**@brief Get the index of a face known by its handle.
     *
     * Get the index of a face known by an handle. If the handle does
     * not point to a valid face, an exception is thrown.
     * @param handle Handle of the face we want the index.
     */
    face_index
    get_index(
      face_handle handle ) const;
    /**@brief Get the index of a face known by a reference.
     *
     * Get the index of a face known by a reference. If the reference does
     * not point to a valid face, an exception is thrown.
     * @param e Reference of the face we want the index.
     */
    face_index
    get_index(
      face& e ) const;
    /**@brief Get the handle of a face.
     *
     * Get the handle of a face. If the face points to memory that is
     * not tagged as a valid face, an exception is thrown.
     * @param e Reference to the face we want to know the handle.
     */
    face_handle
    get_handle(
      face& e ) const;
    /**@brief Check if a face handle is valid.
     *
     * An face handle is valid if it identifies a currently allocated
     * face in the tight buffer. This function check if an handle is
     * valid.
     * @param handle The handle to check.
     */
    bool
    is_valid(
      face_handle handle ) const;

    /**@brief Apply a process function on all faces.
     *
     * This method apply, in parallel or not, a function on each
     * valid face.
     * @param function The function to apply.
     * @param parallel A flag to activate a parallel processing.
     */
    template< typename face_processer >
      void
      process_faces(
        face_processer&& function, bool parallel = true );

    /**@brief Filter faces according to a filter function.
     *
     * This method select faces to remove thanks to a filter function. The
     * evaluation of the filter function can be done in parallel. This method
     * guarantees that the faces are still stored in a tight buffer after
     * removals. This function will invalidate references and indices to faces.
     * @param filter The filter function used to select faces to remove.
     * @param parallel A flag to activate a parallel evaluation of the filter function.
     */
    template< typename face_filter >
      void
      remove_faces(
        face_filter&& filter, bool parallel = true );

  private:

    void
    remove_atom_special_properties(
      atom_index idx );

    link_handle
    do_add_link(
      atom_index idx1, atom_index idx2 );
    void
    remove_link_topology_properties(
      link_index idx, link_handle handle );
    void
    remove_link_indices(
      bool* flags );

    face_handle
    do_add_face(
      atom_index idx1, atom_index idx2, atom_index idx3 );

    /**@brief Remove all references to a face in atom and links topology properties.
     *
     * Remove all atom_face_element and link_face_element referring to the face
     * with the specified index and handle.
     * @param idx Index of the face.
     * @param handle Handle of the face.
     */
    void
    remove_face_topology_properties(
      face_index idx, face_handle handle );
    /**@brief Remove all tagged faces.
     *
     * This function guarantees to remove each face with index i such that
     * flags[i] is true, while keeping the whole skeleton in a valid state.
     * In particular, the face buffer remains tight, atom_faces_property
     * and link_faces_property are updated.
     * @param flags Identify the indices of faces to remove. */
    void
    remove_faces_indices(
      bool* flags );

    void
    remove_link_to_face(
      link_index idx, face_handle handle );
    void
    remove_atom_to_face(
      atom_index idx, face_handle handle );
    void
    remove_atom_to_link(
      atom_index idx, link_handle handle );

    datastructure* m_impl;
  };

END_MP_NAMESPACE
# include "detail/median_skeleton.tcc"
# endif 
