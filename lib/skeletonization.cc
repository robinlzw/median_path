/*  Created on: Mar 15, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "../median-path/skeletonization.h"
# include <graphics-origin/tools/log.h>
# include <omp.h>
BEGIN_MP_NAMESPACE

  void shrinking_ball_skeletonizer(
      graphics_origin::geometry::mesh_spatial_optimization& input,
      median_skeleton& output,
      const skeletonizer::parameters& params );

  void voronoi_ball_skeletonizer(
    graphics_origin::geometry::mesh_spatial_optimization& input,
    median_skeleton& output,
    const skeletonizer::parameters& params );

  void polar_ball_skeletonizer(
    graphics_origin::geometry::mesh_spatial_optimization& input,
    median_skeleton& output,
    const skeletonizer::parameters& params );

  void regular_triangulation_reconstruction(
      median_skeleton& output,
      const structurer::parameters& params );

  skeletonizer::parameters::parameters()
    : m_geometry_method{ SHRINKING_BALLS },
      m_cluster_volume_factor{ 0.005 }, m_neighbors_for_cluster_detection{ 10 },
      m_merge_clusters{ true },
      m_shrinking_ball{},

      m_build_topology{ true }, m_structurer_parameters{}
  {}

  skeletonizer::skeletonizer(
      graphics_origin::geometry::mesh& input,
      median_skeleton& output,
      const parameters& params )
    : m_execution_time{ omp_get_wtime() }
  {
    graphics_origin::geometry::mesh_spatial_optimization mso( input, false, false );
    switch( params.m_geometry_method )
    {
      case parameters::SHRINKING_BALLS:
        shrinking_ball_skeletonizer( mso, output, params );
        break;

      case parameters::VORONOI_BALLS:
        voronoi_ball_skeletonizer( mso, output, params );
        break;

      case parameters::POLAR_BALLS:
        polar_ball_skeletonizer( mso, output, params );
        break;
    }

    switch( params.m_structurer_parameters.m_topology_method )
    {
      case structurer::parameters::WEIGHTED_ALPHA_SHAPE:
        regular_triangulation_reconstruction( output, params.m_structurer_parameters );
        break;
      case structurer::parameters::DELAUNAY_RECONSTRUCTION:
        LOG( debug, "not available yet");
        break;

      case structurer::parameters::POWERSHAPE:
        if( params.m_geometry_method != parameters::POLAR_BALLS &&
            params.m_geometry_method != parameters::VORONOI_BALLS )
          {
            LOG( error, "cannot have a powershape reconstruction if the geometry method is neither polar balls nor voronoi balls");
          }
        break;
      case structurer::parameters::VORONOI:
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
