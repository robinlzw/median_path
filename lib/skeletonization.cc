/*  Created on: Mar 15, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "../median-path/skeletonization.h"

# include <omp.h>
BEGIN_MP_NAMESPACE

  extern
  void shrinking_ball_skeletonizer(
      graphics_origin::geometry::mesh& input,
      median_skeleton& output,
      const parameters& params );

  extern
  void shrinking_ball_skeletonizer(
    vec3* input_points,
    vec3* input_normals,
    size_t number_of_samples,
    median_skeleton& output,
    const parameters& params );

  skeletonizer::parameters::parameters()
    : m_geometry_method{ SHRINKING_BALLS },
      m_topology_method{ REGULAR_TRIANGULATION },
      m_cluster_volume_factor{ 0.005 }, m_neighbors_for_cluster_detection{ 10 },
      m_build_topology{ true }, m_build_faces{ true }, m_merge_clusters{ true }
  {}

  skeletonizer::skeletonizer(
      graphics_origin::geometry::mesh& input,
      median_skeleton& output,
      const parameters& params )
  {
    m_execution_time = omp_get_wtime();
    switch( params.m_geometry_method )
    {
      case parameters::SHRINKING_BALLS:
        shrinking_ball_skeletonizer( input, output, params );
        break;
      default:
        LOG( debug, "not all case are implemented");
    }
    m_execution_time = omp_get_wtime() - m_execution_time;
  }

  skeletonizer::skeletonizer(
      vec3* input_points,
      vec3* input_normals,
      size_t number_of_samples,
      median_skeleton& output,
      const parameters& params )
  {
    m_execution_time = 0;
    LOG( debug, "TODO");
  }


END_MP_NAMESPACE
