/* Created on: Mar 25, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# ifndef MEDIAN_PATH_STRUCTURATION_H_
# define MEDIAN_PATH_STRUCTURATION_H_

# include "median_skeleton.h"

BEGIN_MP_NAMESPACE

/**@brief Structure a median skeleton.
 *
 * This class structures a median skeleton, that is it reconstruct the topology
 * of the shape described by the atom in the form of links and triangles
 * connecting skeleton points.
 *
 * Currently, 4 methods are available, but not all of them can be applied apart
 * from a skeletonization algorithm:
 * - weighted alpha shape, can be applied on any median skeleton, at any time
 * - delaunay reconstruction, can be applied on any median skeleton, at any time
 * - powershape, should be applied during Voronoi or polar skeletonization only
 * - voronoi, should be applied during Voronoi skeletonization only. */
class structurer {
public:

  /**@brief Parameters to tune a structuration.
   *
   * Parameters of a structuration process. */
  struct parameters {
    parameters();
    typedef enum {
      WEIGHTED_ALPHA_SHAPE,
      DELAUNAY_RECONSTRUCTION,
      POWERSHAPE,
      VORONOI
    } topology_method;
    topology_method m_topology_method;
    bool m_build_faces;
    bool m_neighbors_should_intersect;
  };

  structurer(
      median_skeleton& skeleton,
      const parameters& params = parameters() );

  const real& get_execution_time() const noexcept;
private:
  real m_execution_time;
};

END_MP_NAMESPACE
# endif 
