/* Created on: Mar 1, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# ifndef MEDIAN_PATH_MEDIAN_SKELETON_H_
# define MEDIAN_PATH_MEDIAN_SKELETON_H_

# include "median_path.h"
# include <graphics-origin/tools/tight_buffer_manager.h>
# include <graphics-origin/geometry/vec.h>



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

  typedef GO_NAMESPACE::tools::tight_buffer_manager<
    atom,
    uint32_t,
    22 > atom_manager;

  struct link {
    atom_manager::handle a;
    atom_manager::handle b;
  };

  typedef GO_NAMESPACE::tools::tight_buffer_manager<
      link,
      uint64_t,
      44 > link_manager;

  /**
   *================================================================================
   * A link is useful for three reasons:
   * 1. rendering a line that connects its end points
   * 2. getting the neighbors of an atom
   * 3. storing link data.
   *
   * 1. could be done with a) atom indices or b) by sending duplicated vertices.
   * a) require to recompute the indices buffer at each atom removal but also require
   * less data to send to the GPU. In any cases, we need the atom indices to fill
   * the vertex index buffer or to duplicate atoms.
   *
   * 2.
   *  we just need the atom indices. This will need to be computed each time
   * after an atom removal or we would send duplicated vertices.
   *
   * A link could be identified by the handles of its atoms:
   * l =  counter||a1.index|a2.index
   *                22 bits  22 bits
   * However, how to go from this handle to the index?
   * Indeed, the index part of the handle does not refer to a place in an entry buffer.
   *
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
   *   an atom is basically a vec4 augmented with some methods
   *   This is the data that will be the more often used and transferred (e.g.:
   *   to the gpu for rendering or computing)
   *
   * - links
   *   a link is a connection between two atoms. How to represent that?
   *   Well, we have to keep in mind that links could be added and removed.
   *   Also, we do not want to have an adjacency matrix for that since a)
   *   the number of atoms could be pretty high and b) atoms will be added
   *   and removed too. Thus, we should store something similar to an adjacency
   *   list. Due to the link removal features, it is not practical to store
   *   all links of the same atom contiguously in only one buffer. Each atom
   *   will store its neighbors in a buffer of vectors.
   *   Still, each link should be identified by an handle to be able to index
   *   link data. It is tempting to use the index part of the two atom handles
   *   to build a link handle. However, in this case we could not use the
   *   index part of the link handle to access to a buffer. We need then an
   *   handle entry buffer (that maps the index of an handle to the index of
   *   a link data in a tight buffer).
   *
   * - faces
   *   I consider only triangular faces.
   *
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
   *
   */


  class median_skeleton {
  public:





  private:
    atom_manager m_atoms;
  };

END_MP_NAMESPACE
# endif 
