/*  Created on: Mar 14, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */

# ifndef MEDIAN_PATH_SKELETONIZATION_H_
# define MEDIAN_PATH_SKELETONIZATION_H_

# include "median_skeleton.h"
# include <graphics-origin/geometry/mesh.h>

BEGIN_MP_NAMESPACE

  class skeletonizer {
  public:

    struct shrinking_ball_parameters {
      shrinking_ball_parameters();

      /**@brief How to determine the initial radius.
       *
       * Currently, there is two methods to determine the initial radius of
       * shrinking balls:
       * - cast a ray from a vertex in the opposite direction of the normal
       * and use the distance to the intersection as the initial value
       * - use a constant value expressed relatively to the smallest dimension
       * of the bounding box.
       */
      typedef enum {
        RAYTRACING, /*!< Cast a ray in the opposite direction of the normal */
        CONSTANT    /*!< Use always the same initial value.*/
      } initial_radius_method;

      initial_radius_method m_radius_method;

      /**@brief Ratio of the bounding box used to compute the constant initial radius.
       *
       * This value stores the ratio of the smallest dimension of the bounding box
       * to use as a constant initial radius. */
      real m_constant_radius_ratio;
      /**@brief Radius variation threshold.
       *
       * When a shrinking ball radius does not change more than this threshold,
       * the ball stop to shrinks. This value is used to avoid infinite loop due
       * to the numerical precision. */
      real m_min_radius_variation;
    };

    struct parameters {

      typedef enum {
        SHRINKING_BALLS,
        POLAR_BALLS,
        VORONOI_BALLS
      } geometry_method;

      typedef enum {
        REGULAR_TRIANGULATION,
        DELAUNAY_RECONSTRUCTION,
        POWERSHAPE
      } topology_method;

      parameters();

      geometry_method m_geometry_method;
      topology_method m_topology_method;
      real m_cluster_volume_factor;
      median_skeleton::atom_index  m_neighbors_for_cluster_detection;

      bool m_build_topology;
      bool m_build_faces;
      bool m_merge_clusters;

      union {
        shrinking_ball_parameters m_shrinking_ball;
      };
    };

    skeletonizer(
        graphics_origin::geometry::mesh& input,
        median_skeleton& output,
        const parameters& params = parameters() );

    real get_execution_time() const noexcept;

  private:
    real m_execution_time;
  };

END_MP_NAMESPACE

# endif
