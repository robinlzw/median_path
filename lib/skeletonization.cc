/*  Created on: Mar 15, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "../median-path/skeletonization.h"
# include <valgrind/callgrind.h>
# include <omp.h>
BEGIN_MP_NAMESPACE

  extern
  void shrinking_ball_skeletonizer(
      graphics_origin::geometry::mesh_spatial_optimization& input,
      median_skeleton& output,
      const skeletonizer::parameters& params );

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
    // take into account the construction of this structure in the execution time
    graphics_origin::geometry::mesh_spatial_optimization mso( input, false, false );
    LOG( debug, "took " << omp_get_wtime() - m_execution_time << " to build SMO");
    switch( params.m_geometry_method )
    {
      case parameters::SHRINKING_BALLS:
        {
        CALLGRIND_START_INSTRUMENTATION;
        shrinking_ball_skeletonizer( mso, output, params );
        CALLGRIND_STOP_INSTRUMENTATION;
        }
        break;
      default:
        LOG( debug, "not all case are implemented");
    }
    m_execution_time = omp_get_wtime() - m_execution_time;
  }

  real
  skeletonizer::get_execution_time() const noexcept
  {
    return m_execution_time;
  }


END_MP_NAMESPACE
