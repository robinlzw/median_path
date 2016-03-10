/* Created on: Mar 1, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# ifndef MEDIAN_PATH_MEDIAN_SKELETON_H_
# define MEDIAN_PATH_MEDIAN_SKELETON_H_

# include "detail/skeleton_datastructure.h"
# include <graphics-origin/geometry/vec.h>

# include <string>
# include <stdint.h>
# include <functional>
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
   * handles, references or indices. Handles are safer way to identify elements,
   * but is also slower. References and indices are faster, but could throw
   * exception if they are incorrect. Indeed, both references and indices are
   * subject to change. Thus, you might have references or indices that were
   * correct at the moment you obtained them, but operations that happened
   * after invalidated them. Adding and removing elements are the operations that
   * invalidate references and indices.
   *
   * Maximum capacities:
   *   - 2^22 atoms
   *   - 2^44 links
   *   - 2^54 faces
   */
  class median_skeleton {
    typedef skeleton_datastructure<
        uint32_t, 22,
        uint64_t, 44,
        uint64_t, 54 > datastructure;
  public:
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
    struct atom_face_element {
      link_handle links[3];
      atom_handle atoms[2];
      face_handle face;

      atom_face_element()
        : links{ link_handle{}, link_handle{}, link_handle{} },
          atoms{ atom_handle{}, atom_handle{} },
          face{ face_handle{} }
      {}

      atom_face_element&
      operator=( atom_face_element&& other )
      {
        links[0] = other.links[0]; links[1] = other.links[1]; links[2] = other.links[2];
        atoms[0] = other.atoms[0]; atoms[1] = other.atoms[1];
        face = other.face;
        return *this;
      }
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
    struct link_face_element {
      link_handle links[2];
      atom_handle opposite;
      face_handle face;

      link_face_element()
        : links{ link_handle{}, link_handle{} },
          opposite{ atom_handle{} },
          face{ face_handle{} }
      {}

      link_face_element&
      operator=( link_face_element&& other )
      {
        links[0] = other.links[0]; links[1] = other.links[1];
        opposite = other.opposite;
        face = other.face;
        return *this;
      }
    };

    typedef std::vector< std::pair<link_handle, atom_handle> > atom_links_property;
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
      atom_index atom_capacity = 0,
      uint64_t link_capacity = 0,
      uint64_t face_capacity = 0 );
    /**@brief Copy constructor
     *
     * Copy a skeleton into this. This operation is very fast as it is merely
     * a swap between two pointers. At the end of the copy, the other skeleton
     * is in a undefined, but valid, state. */
    median_skeleton( median_skeleton&& other);
    /**@brief Build from file
     *
     * Build a median skeleton from a file.
     * @param filename The path of the file describing the skeleton to build. */
    median_skeleton( const std::string& filename );

    ~median_skeleton();
    ///@}

    /**@name Utilities
     * @{ */
    /**@brief Clear the internal structures of the skeleton.
     *
     * After the call, the skeleton is empty but with enough space to store the
     * requested number of atom, link and faces. The previous atom, link and
     * face properties still exist in the skeleton but are now empty. The result
     * would be the same if we build another skeleton with the requested capacities
     * and copy it into this.
     * @param atom_capacity Requested atom capacity
     * @param link_capacity Requested link capacity
     * @param face_capacity Requested face capacity */
    void clear(
      atom_index atom_capacity = 0,
      link_index link_capacity = 0,
      face_index face_capacity = 0 );

    /**@brief Load a skeleton from a file.
     *
     * Load a median skeleton described by a file into this skeleton.
     * @param filename The path of the file describing the skeleton to build.
     * @return true if the load was successful */
    bool load( const std::string& filename );

    /**@brief Save a skeleton to a file.
     *
     * Save this skeleton to a file.
     * @param filename The path of the file to write this skeleton into.
     * @return true if the save was successful */
    bool save( const std::string& filename );

    /**@brief Get the current number of atoms */
    atom_index get_number_of_atoms() const noexcept;
    /**@brief Get the current number of links */
    link_index get_number_of_links() const noexcept;
    /**@brief Get the current number of faces */
    face_index get_number_of_faces() const noexcept;

    /**@name Atom management
     * @{ */
    /**@brief Add an atom to the skeleton.
     *
     * Add an atom to this skeleton with the center and the radius
     * sent separately.
     * @param position Center of the atom to add.
     * @param radius Radius of the atom to add.
     * @return An atom handle that points to the newly created atom. */
    atom_handle add( const vec3& position, const real& radius );
    /**@brief Add an atom to the skeleton.
     *
     * Add an atom to this skeleton with its geometry stored in a
     * vec4.
     * @param ball A vec4 with xyz describing the atom center and w
     * storing the radius.
     * @return An atom handle that points to the newly created atom. */
    atom_handle add( const vec4& ball );

    /**@brief Remove an atom know by its handle.
     *
     * Remove an atom from the skeleton. Its faces and links will
     * be destroyed too. If the handle is invalid (e.g. the atom
     * is already removed), the error is silently ignored.
     * @param handle Handle of the atom to remove.
     */
    void remove( atom_handle handle );

    /**@brief Remove an atom known by a reference.
     *
     * Remove an atom from the skeleton. Its faces and links will
     * be destroyed too. If the atom points to memory that is not
     * tagged as a valid atom, an exception is thrown.
     * @param e Reference to the atom to remove.
     */
    void remove( atom& e );

    /**@brief Access to an atom known by an handle.
     *
     * Access to an atom thank to its handle. If the handle is
     * incorrect, an exception is thrown. Be careful to not store this
     * reference if you plan to add/remove atoms after, since it will
     * invalidate the reference.
     * @param handle Handle of the atom in the tight buffer.
     */
    atom& get( atom_handle handle ) const;

    /**@brief Access to an atom known by an index.
     *
     * Access to an atom thank to its index. If the index is
     * incorrect, an exception is thrown. Be careful to not store this
     * reference if you plan to add/remove atoms after, since it will
     * invalidate the reference.
     * @param index Index of the atom in the tight buffer.
     */
    atom& get_atom_by_index( atom_index index ) const;

    /**@brief Get the index of an atom known by its handle.
     *
     * Get the index of an atom known by an handle. If the handle does
     * not point to a valid atom, an exception is thrown.
     * @param handle Handle of the atom we want the index.
     */
    atom_index get_index( atom_handle handle ) const;
    /**@brief Get the handle of an atom.
     *
     * Get the handle of an atom. If the atom points to memory that is
     * not tagged as a valid atom, an exception is thrown.
     * @param e Reference to the atom we want to know the handle.
     */
    atom_handle get_handle( atom& e ) const;
    /**@brief Check if an atom handle is valid.
     *
     * An atom handle is valid if it identifies a currently allocated
     * atom in the tight buffer. This function check if an handle is
     * valid.
     * @param handle The handle to check.
     */
    bool is_valid( atom_handle handle ) const;

    /**@brief A function to process atoms.
     *
     * A function of such a type can be applied on all valid atoms of a skeleton.
     */
    typedef std::function<void(atom&)> atom_processer;

    /**@brief Apply a process function on all atoms.
     *
     * This method apply, in parallel or not, a function on each
     * valid atom.
     * @param function The function to apply.
     * @param parallel A flag to activate a parallel processing.
     */
    void process( atom_processer&& function, bool parallel = true );

    /**@brief A function to select atoms to remove.
     *
     * A function of such a type is used to identify atoms to remove from a
     * skeleton.
     */
    typedef std::function<bool(atom&)> atom_filter;
    /**@brief Filter atoms according to a filter function.
     *
     * This method select atoms to remove thanks to a filter function. The
     * evaluation of the filter function can be done in parallel. This method
     * guarantees that the atoms are still stored in a tight buffer after
     * removals. This function will invalidate references and indices to atoms.
     * @param filter The filter function used to select atoms to remove.
     * @param parallel A flag to activate a parallel evaluation of the filter function.
     */
    void remove( atom_filter&& filter, bool parallel = true );

    /**@name Link management
     * @{ */
    /**@brief Add an link to the skeleton.
     *
     * Add an link between two atoms. If one of the handles is invalid,
     * an exception is thrown. If the link already exists, its handle is returned.
     * @param handle1 The handle of the first atom.
     * @param handle2 The handle of the second atom.
     * @return An link handle that points to the newly created link. */
    link_handle add( atom_handle handle1, atom_handle handle2 );
    /**@brief Add an link to the skeleton.
     *
     * Add an link between two atoms. If one of the atom references is invalid,
     * an exception is thrown. If the link already exists, its handle is returned.
     * @param atom1 Reference to the first atom.
     * @param atom2 Reference to the second atom.
     * @return An link handle that points to the newly created link. */
    link_handle add( atom& atom1, atom& atom2 );
    /**@brief Add an link to the skeleton.
     *
     * Add an link between two atoms. If one of the atom indices is invalid,
     * an exception is thrown. If the link already exists, its handle is
     * returned.
     * @param idx1 Index of the first atom.
     * @param idx2 Index of the second atom.
     * @return An link handle that points to the newly created link. */
    link_handle add( atom_index idx1, atom_index idx2 );
    /**@brief Remove an link know by its handle.
     *
     * Remove an link from the skeleton. Its faces will
     * be destroyed too. Also, its atoms will be notified that they are
     * no longer connected by the corresponding link and faces.
     * If the handle is invalid (e.g. the link
     * is already removed), the error is silently ignored.
     * @param handle Handle of the link to remove.
     */
    void remove( link_handle handle );

    /**@brief Remove an link known by a reference.
     *
     * Remove an link from the skeleton. Its faces will
     * be destroyed too. Also, its atoms will be notified that they are
     * no longer connected by the corresponding link and faces.
     * If the link points to memory that is not
     * tagged as a valid link, an exception is thrown.
     * @param e Reference to the link to remove.
     */
    void remove( link& e );

    /**@brief Access to an link known by an handle.
     *
     * Access to an link thank to its handle. If the handle is
     * incorrect, an exception is thrown. Be careful to not store this
     * reference if you plan to add/remove links after, since it will
     * invalidate the reference.
     * @param handle Handle of the link in the tight buffer.
     */
    link& get( link_handle handle ) const;

    /**@brief Access to an link known by an index.
     *
     * Access to an link thank to its index. If the index is
     * incorrect, an exception is thrown. Be careful to not store this
     * reference if you plan to add/remove links after, since it will
     * invalidate the reference.
     * @param index Index of the link in the tight buffer.
     */
    link& get_link_by_index( link_index index ) const;

    /**@brief Get the index of an link known by its handle.
     *
     * Get the index of an link known by an handle. If the handle does
     * not point to a valid link, an exception is thrown.
     * @param handle Handle of the link we want the index.
     */
    link_index get_index( link_handle handle ) const;
    /**@brief Get the handle of an link.
     *
     * Get the handle of an link. If the link points to memory that is
     * not tagged as a valid link, an exception is thrown.
     * @param e Reference to the link we want to know the handle.
     */
    link_handle get_handle( link& e ) const;
    /**@brief Check if an link handle is valid.
     *
     * An link handle is valid if it identifies a currently allocated
     * link in the tight buffer. This function check if an handle is
     * valid.
     * @param handle The handle to check.
     */
    bool is_valid( link_handle handle ) const;

    /**@brief A function to process links.
     *
     * A function of such a type can be applied on all valid links of a skeleton.
     */
    typedef std::function<void(link&)> link_processer;

    /**@brief Apply a process function on all links.
     *
     * This method apply, in parallel or not, a function on each
     * valid link.
     * @param function The function to apply.
     * @param parallel A flag to activate a parallel processing.
     */
    void process( link_processer&& function, bool parallel = true );

    /**@brief A function to select links to remove.
     *
     * A function of such a type is used to identify links to remove from a
     * skeleton.
     */
    typedef std::function<bool(link&)> link_filter;
    /**@brief Filter links according to a filter function.
     *
     * This method select links to remove thanks to a filter function. The
     * evaluation of the filter function can be done in parallel. This method
     * guarantees that the links are still stored in a tight buffer after
     * removals. This function will invalidate references and indices to links.
     * @param filter The filter function used to select links to remove.
     * @param parallel A flag to activate a parallel evaluation of the filter function.
     */
    void remove( link_filter&& filter, bool parallel = true );

  private:

    void remove_link_to_face( link_index idx, face_handle handle );
    void remove_atom_to_face( atom_index idx, face_handle handle );
    void remove_atom_to_link( atom_index idx, link_handle handle );

    datastructure* m_impl;
  };

END_MP_NAMESPACE
# endif 
