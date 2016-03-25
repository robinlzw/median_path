/*  Created on: Mar 20, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "../median-path/skeletonization.h"
# include <graphics-origin/tools/log.h>

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
  struct voronoi_ball {
    voronoi_ball()
      : atom{}, idx{0}, is_inside{ false }
    {}

    voronoi_ball( voronoi_ball& other )
      : atom{ other.atom }, idx{ other.idx }, is_inside{ other.is_inside }
    {}

    voronoi_ball&
    operator=( voronoi_ball& other )
    {
      atom = other.atom;
      idx = other.idx;
      is_inside = other.is_inside;
      return *this;
    }
    median_skeleton::atom atom;
    median_skeleton::atom_index idx;
    bool is_inside;
  };

  typedef CGAL::Triangulation_vertex_base_with_info_3< dt_vertex_info, dt_kernel > dt_vertex_base;
  typedef CGAL::Triangulation_cell_base_with_info_3< voronoi_ball*, dt_kernel > dt_cell_base;
  typedef CGAL::Triangulation_data_structure_3< dt_vertex_base, dt_cell_base, CGAL::Parallel_tag > dt_datastructure;
  typedef CGAL::Delaunay_triangulation_3< dt_kernel, dt_datastructure > dt;


  void voronoi_structuration(
      median_skeleton& skeleton,
      dt& delaunay_tetrahedrization,
      const structurer::parameters& params )
  {
    if( params.m_build_faces )
      {
        skeleton.reserve_faces( delaunay_tetrahedrization.number_of_finite_edges() );
        skeleton.reserve_links( delaunay_tetrahedrization.number_of_finite_facets() );
        for( auto feit = delaunay_tetrahedrization.finite_edges_begin(),
            feend = delaunay_tetrahedrization.finite_edges_end(); feit != feend;
            ++ feit )
          {
            auto circulator = delaunay_tetrahedrization.incident_cells( *feit );
            std::vector< voronoi_ball* > vballs;
            auto begin = circulator;
            bool ok = true;
            do
              {
                auto info = circulator->info();
                if( !info || !info->is_inside )
                  {
                    ok = false;
                    break;
                  }
                vballs.push_back( info );
                ++circulator;
              }
            while( circulator != begin );

            size_t nelements = vballs.size();
            if( ok && nelements > 2 )
              {
                if( params.m_neighbors_should_intersect )
                  {
                    size_t start = 0;
                    for( size_t i = 1; i < nelements; ++ i )
                      {
                        if( vballs[0]->atom.intersect( vballs[i]->atom ) )
                          {
                            start = i;
                            break;
                          }
                      }
                    for( size_t i = start + 1; i < nelements; ++ i )
                      {
                        if( vballs[i]->atom.intersect( vballs[0]->atom )
                         && vballs[i]->atom.intersect( vballs[i-1]->atom ) )
                         {
                            skeleton.add( vballs[0]->idx, vballs[i-1]->idx, vballs[i]->idx );
                         }
                      }
                  }
                else
                  {

                    for( size_t i = 2; i < nelements; ++ i )
                      {
                        skeleton.add( vballs[0]->idx, vballs[i-1]->idx, vballs[i]->idx );
                      }
                  }
              }
          }
      }
//    else
      {
//        skeleton.reserve_links( delaunay_tetrahedrization.number_of_finite_facets() );
        for( auto ffit = delaunay_tetrahedrization.finite_facets_begin(),
            ffend = delaunay_tetrahedrization.finite_facets_end(); ffit != ffend;
            ++ ffit )
          {
            auto pi1 = ffit->first->info();
            auto pi2 = ffit->first->neighbor( ffit->second )->info();

            if( pi1 && pi2 && pi1->is_inside && pi2->is_inside &&
                (!params.m_neighbors_should_intersect || pi1->atom.intersect( pi2->atom ) ) )
              skeleton.add( pi1->idx, pi2->idx );
          }
      }
  }

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

    voronoi_ball* voronoi_balls = new voronoi_ball[ delaunay_tetrahedrisation.number_of_finite_cells() ];
    size_t voronoi_ball_index = 0;
    for( auto cit = delaunay_tetrahedrisation.finite_cells_begin(),
        end = delaunay_tetrahedrisation.finite_cells_end(); cit != end; ++ cit )
      {
        auto& info = voronoi_balls[ voronoi_ball_index ];
        auto voronoi_vertex = delaunay_tetrahedrisation.dual( cit );
        info.atom = vec4 {
          CGAL::to_double( voronoi_vertex.x() ),
          CGAL::to_double( voronoi_vertex.y() ),
          CGAL::to_double( voronoi_vertex.z() ),
          CGAL::to_double(CGAL::squared_distance( cit->vertex(0)->point(), voronoi_vertex ))
        };
        cit->info() = &info;
        ++voronoi_ball_index;
      }

    size_t atom_index = 0;
    # pragma omp parallel for
    for( size_t i = 0; i < voronoi_ball_index; ++ i )
    {
        auto& info = voronoi_balls[ i ];
        if( input.contain( vec3(info.atom) ) )
          {
            info.is_inside = true;
            info.atom.w = std::sqrt( info.atom.w );
            # pragma omp critical
            {
              output.add( info.atom );
              info.idx = atom_index;
              ++atom_index;
            }
          }
      }

    if( params.m_build_topology )
      {
        if( params.m_structurer_parameters.m_topology_method == structurer::parameters::VORONOI )
          {
            voronoi_structuration( output, delaunay_tetrahedrisation, params.m_structurer_parameters );
          }
        else if( params.m_structurer_parameters.m_topology_method == structurer::parameters::POWERSHAPE )
          {

          }
      }
    delete[] voronoi_balls;
  }



END_MP_NAMESPACE
