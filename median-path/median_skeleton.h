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

  /**
   * atom,
   * link,
   * face
   *
   * atom data:
   *   [0] std::vector< link_handle >
   *   [1] std::vector< face_handle >
   * link data
   *   [0] std::vector< face_handle >
   * face data
   *
   * For most methods, you have the choice to identify skeleton elements by
   * handles, references or indices. Handles are safer way to identify elements,
   * but is also slower. References and indices are faster, but could throw
   * exception if they are incorrect. Indeed, both references and indices are
   * subject to change. Thus, you might have references or indices that were
   * correct at the moment you obtained them, but operations that happened
   * after invalidated them.
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

    typedef std::vector< link_handle > atom_links_property;
    typedef std::vector< face_handle > atom_faces_property;
    typedef std::vector< face_handle > link_faces_property;

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
      uint32_t atom_capacity = 0,
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
      uint32_t atom_capacity = 0,
      uint64_t link_capacity = 0,
      uint64_t face_capacity = 0 );

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
    uint32_t get_number_of_atoms() const noexcept;
    /**@brief Get the current number of links */
    uint64_t get_number_of_links() const noexcept;
    /**@brief Get the current number of faces */
    uint64_t get_number_of_faces() const noexcept;

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
    atom& get_atom_by_index( uint32_t index ) const;

    /**@brief Get the index of an atom known by its handle.
     *
     * Get the index of an atom known by an handle. If the handle does
     * not point to a valid atom, an exception is thrown.
     * @param handle Handle of the atom we want the index.
     */
    uint32_t get_index( atom_handle handle ) const;
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

    typedef std::function<void(atom&)> atom_processer;
    typedef std::function<bool(atom&)> atom_filter;
  private:
    datastructure* m_impl;
  };

END_MP_NAMESPACE
# endif 
