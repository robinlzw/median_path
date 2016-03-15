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
    };

    skeletonizer(
        graphics_origin::geometry::mesh& input,
        median_skeleton& output,
        const parameters& params = parameters() );

    skeletonizer(
        vec3* input_points,
        vec3* input_normals,
        size_t number_of_samples,
        median_skeleton& output,
        const parameters& params = parameters() );

    real get_execution_time() const noexcept;

  private:
    real m_execution_time;
  };

END_MP_NAMESPACE

# endif
