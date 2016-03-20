/*  Created on: Mar 20, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "../median-path/skeletonization.h"

# define CGAL_LINKED_WITH_TBB
# include <tbb/task_scheduler_init.h>
# include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
# include <CGAL/Exact_predicates_exact_constructions_kernel.h>
# include <CGAL/Triangulation_vertex_base_with_info_3.h>
# include <CGAL/Triangulation_cell_base_with_info_3.h>
# include <CGAL/Location_policy.h>
# include <CGAL/Delaunay_triangulation_3.h>


BEGIN_MP_NAMESPACE

  typedef uint32_t dt_vertex_info;

  typedef CGAL::Epeck dt_kernel;

  typedef CGAL::Triangulation_vertex_base_with_info_3< dt_vertex_info, dt_kernel > dt_vertex_base;
  typedef CGAL::Triangulation_cell_base_3< dt_kernel > dt_cell_base; //_with_info_3< dt_cell_info, dt_kernel > dt_cell_base;
  typedef CGAL::Triangulation_data_structure_3< dt_vertex_base, dt_cell_base, CGAL::Parallel_tag > dt_datastructure;
  typedef CGAL::Delaunay_triangulation_3< dt_kernel, dt_datastructure > dt;


  static void add_extended_bounding_box_vertices(
      dt& delaunay_tetrahedrisation,
      const graphics_origin::geometry::aabox& bbox )
  {
    vec3 point_min = bbox.get_min();
    vec3 point_max = bbox.get_max();
    dt::Vertex_handle v1 = delaunay_tetrahedrisation.insert ( dt::Point ( point_min.x, point_min.y, point_min.z ) );
    dt::Vertex_handle v2 = delaunay_tetrahedrisation.insert ( dt::Point ( point_min.x, point_min.y, point_max.z ) );
    dt::Vertex_handle v3 = delaunay_tetrahedrisation.insert ( dt::Point ( point_min.x, point_max.y, point_min.z ) );
    dt::Vertex_handle v4 = delaunay_tetrahedrisation.insert ( dt::Point ( point_min.x, point_max.y, point_max.z ) );
    dt::Vertex_handle v5 = delaunay_tetrahedrisation.insert ( dt::Point ( point_max.x, point_min.y, point_min.z ) );
    dt::Vertex_handle v6 = delaunay_tetrahedrisation.insert ( dt::Point ( point_max.x, point_min.y, point_max.z ) );
    dt::Vertex_handle v7 = delaunay_tetrahedrisation.insert ( dt::Point ( point_max.x, point_max.y, point_min.z ) );
    dt::Vertex_handle v8 = delaunay_tetrahedrisation.insert ( dt::Point ( point_max.x, point_max.y, point_max.z ) );

//    v1->info() = null_dt_vertex_info;
//    v2->info() = null_dt_vertex_info;
//    v3->info() = null_dt_vertex_info;
//    v4->info() = null_dt_vertex_info;
//    v5->info() = null_dt_vertex_info;
//    v6->info() = null_dt_vertex_info;
//    v7->info() = null_dt_vertex_info;
//    v8->info() = null_dt_vertex_info;

//    std::list< dt::Cell_handle > cells_outside_convex_hull;
//    delaunay_tetrahedrisation.incident_cells( v1, std::back_inserter( cells_outside_convex_hull ) );
//    delaunay_tetrahedrisation.incident_cells( v2, std::back_inserter( cells_outside_convex_hull ) );
//    delaunay_tetrahedrisation.incident_cells( v3, std::back_inserter( cells_outside_convex_hull ) );
//    delaunay_tetrahedrisation.incident_cells( v4, std::back_inserter( cells_outside_convex_hull ) );
//    delaunay_tetrahedrisation.incident_cells( v5, std::back_inserter( cells_outside_convex_hull ) );
//    delaunay_tetrahedrisation.incident_cells( v6, std::back_inserter( cells_outside_convex_hull ) );
//    delaunay_tetrahedrisation.incident_cells( v7, std::back_inserter( cells_outside_convex_hull ) );
//    delaunay_tetrahedrisation.incident_cells( v8, std::back_inserter( cells_outside_convex_hull ) );
//    delaunay_tetrahedrisation.incident_cells( delaunay_tetrahedrisation.infinite_vertex(), std::back_inserter ( cells_outside_convex_hull ) );
//
//    for( auto& c : cells_outside_convex_hull )
//      c->info().m_outside = true;
  }

  struct voronoi_ball {
    vec3 center;
    real radius;
  };

  void voronoi_ball_skeletonizer(
    graphics_origin::geometry::mesh_spatial_optimization& input,
    median_skeleton& output,
    const skeletonizer::parameters& params )
  {
    const auto nsamples = input.kdtree_get_point_count();
    output.clear( nsamples, 0, 0 );
    input.build_bvh();
    input.build_kdtree();

    tbb::task_scheduler_init init;
    std::vector< dt::Point > dtpoints( nsamples );
    std::vector< dt_vertex_info > vinfos( nsamples );
    # pragma omp parallel for schedule(static)
    for( uint32_t i = 0; i < nsamples; ++ i )
      {
        auto p = input.get_point( i );
        dtpoints[ i ] = dt::Point( p[0], p[1], p[2] );
        vinfos[ i ] = i;
      }

    graphics_origin::geometry::aabox bbox = input.get_bounding_box();
    bbox.m_hsides += 1e-6;
    dt::Lock_data_structure locking_datastructure(
        CGAL::Bbox_3(
            bbox.m_center.x - bbox.m_hsides.x, bbox.m_center.y - bbox.m_hsides.y, bbox.m_center.z - bbox.m_hsides.z,
            bbox.m_center.x + bbox.m_hsides.x, bbox.m_center.y + bbox.m_hsides.y, bbox.m_center.z + bbox.m_hsides.z ),
        50);
    dt delaunay_tetrahedrisation(
        boost::make_zip_iterator(boost::make_tuple( dtpoints.begin(), vinfos.begin() )),
        boost::make_zip_iterator(boost::make_tuple( dtpoints.end(), vinfos.end() ) ),
        &locking_datastructure );

    //release now the memory
    std::vector<dt::Point>{}.swap( dtpoints );
    std::vector<dt_vertex_info>{}.swap( vinfos );

    //extend the bounding box and add its vertices to the DT
    //this allows to easily label some outside voronoi balls
    bbox.m_hsides *= 6.0;
    add_extended_bounding_box_vertices( delaunay_tetrahedrisation, bbox );

    // the last time I try to access to cell in parallel, it crashed
    // so now I'm collecting in a single thread voronoi balls, then process them in parallel
    // Note: balls have squared radius for now.
    std::vector< voronoi_ball > vballs( delaunay_tetrahedrisation.number_of_finite_cells());
    const auto number_of_voronoi_balls = vballs.size();
    size_t i = 0;
    for( auto cit = delaunay_tetrahedrisation.finite_cells_begin(),
        end = delaunay_tetrahedrisation.finite_cells_end(); cit != end; ++ cit )
      {
        auto voronoi_vertex = delaunay_tetrahedrisation.dual( cit );
        vballs[ i ].center = vec3 {
          CGAL::to_double( voronoi_vertex.x() ),
          CGAL::to_double( voronoi_vertex.y() ),
          CGAL::to_double( voronoi_vertex.z() )};
        vballs[ i ].radius = CGAL::to_double( CGAL::squared_distance( cit->vertex(0)->point(), voronoi_vertex ) );
        ++i;
      }

    # pragma omp parallel for
    for( i = 0; i < number_of_voronoi_balls; ++ i )
      {
        if( input.contain( vballs[i].center ) )
          {
            if( !input.contain( vballs[i].center ) )
              {
                LOG(debug, "how is this possible?");
              }

            # pragma omp critical
            output.add( vballs[i].center, std::sqrt( vballs[i].radius ) );
          }
      }
  }

END_MP_NAMESPACE
