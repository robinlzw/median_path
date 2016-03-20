/*  Created on: Mar 15, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "../median-path/skeletonization.h"
# include <omp.h>
BEGIN_MP_NAMESPACE

  extern
  void shrinking_ball_skeletonizer(
      graphics_origin::geometry::mesh_spatial_optimization& input,
      median_skeleton& output,
      const skeletonizer::parameters& params );

  void voronoi_ball_skeletonizer(
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
    switch( params.m_geometry_method )
    {
      case parameters::SHRINKING_BALLS:
        shrinking_ball_skeletonizer( mso, output, params );
        break;

      case parameters::VORONOI_BALLS:
        voronoi_ball_skeletonizer( mso, output, params );
        break;
      default:
        LOG( debug, "not all case are implemented");
    }
    switch( params.m_topology_method )
    {
      case parameters::REGULAR_TRIANGULATION:

        break;
      case parameters::DELAUNAY_RECONSTRUCTION:

        break;

      case parameters::POWERSHAPE:
        if( params.m_geometry_method != parameters::POLAR_BALLS &&
            params.m_geometry_method != parameters::VORONOI_BALLS )
          {
            LOG( error, "cannot have a powershape reconstruction if the geometry method is neither polar balls nor voronoi balls");
          }
        break;
      case parameters::VORONOI:
        if( params.m_geometry_method != parameters::VORONOI_BALLS )
          {
            LOG( error, "cannot have a Voronoi reconstruction if the geometry method is not voronoi balls");
          }
    }
    m_execution_time = omp_get_wtime() - m_execution_time;
  }

  real
  skeletonizer::get_execution_time() const noexcept
  {
    return m_execution_time;
  }


END_MP_NAMESPACE
