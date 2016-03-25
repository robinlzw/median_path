/* Created on: Mar 25, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# include "../median-path/structuration.h"

# include <graphics-origin/tools/log.h>

# include <omp.h>
BEGIN_MP_NAMESPACE

void regular_triangulation_reconstruction(
    median_skeleton& output,
    const structurer::parameters& params );

structurer::parameters::parameters()
  : m_topology_method{ WEIGHTED_ALPHA_SHAPE },
    m_build_faces{ true },
    m_neighbors_should_intersect{ true }
{}

structurer::structurer(
    median_skeleton& skeleton,
    const parameters& params )
  : m_execution_time{ omp_get_wtime() }
{
  switch( params.m_topology_method )
  {
    case parameters::WEIGHTED_ALPHA_SHAPE:
      regular_triangulation_reconstruction( skeleton, params );
      break;
    case parameters::DELAUNAY_RECONSTRUCTION:
      LOG( debug, "delaunay reconstruction not available yet");
      break;
    default:
      LOG( error, "only weighted alpha shape and delaunay reconstruction are available apart from a skeletonization.");
  }
  m_execution_time = omp_get_wtime() - m_execution_time;
}

const real&
structurer::get_execution_time() const noexcept
{
  return m_execution_time;
}

END_MP_NAMESPACE
