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
# include <CGAL/Delaunay_triangulation_3.h>
# include <CGAL/Regular_triangulation_euclidean_traits_3.h>
# include <CGAL/Regular_triangulation_3.h>

BEGIN_MP_NAMESPACE

  typedef uint32_t dt_vertex_info;
  static const dt_vertex_info null_dt_vertex_info = uint32_t(-1);

  typedef CGAL::Epeck dt_kernel;
  struct voronoi_ball {

    // status of 0: voronoi ball outside the shape
    // status of 1: voronoi ball inside the shape
    // status of 2: voronoi ball kept for the structuration
    // status of 3: 1 + 2, i.e. inside voronoi ball kept for the structuration, thus an atom
    enum { INSIDE_SHAPE = 1, KEPT = 2, ATOM = 3};



    voronoi_ball()
      : atom{}, idx{0}, status{0}
    {}

    voronoi_ball( const voronoi_ball& other )
      : atom{ other.atom }, idx{ other.idx }, status{ other.status }
    {}

    voronoi_ball&
    operator=( const voronoi_ball& other )
    {
      atom = other.atom;
      idx = other.idx;
      status = other.status;
      return *this;
    }
    median_skeleton::atom atom;
    median_skeleton::atom_index idx;
    int status;
  };

  typedef CGAL::Triangulation_vertex_base_with_info_3< dt_vertex_info, dt_kernel > dt_vertex_base;
  typedef CGAL::Triangulation_cell_base_with_info_3< voronoi_ball*, dt_kernel > dt_cell_base;
  typedef CGAL::Triangulation_data_structure_3< dt_vertex_base, dt_cell_base, CGAL::Parallel_tag > dt_datastructure;
  typedef CGAL::Delaunay_triangulation_3< dt_kernel, dt_datastructure > dt;

  static
  void
  build_and_classify_voronoi_balls(
      graphics_origin::geometry::mesh_spatial_optimization& input,
      median_skeleton& output,
      std::vector< voronoi_ball >& voronoi_balls,
      dt*& delaunay_tetrahedrisation,
      bool extended_bounding_box )
  {
    const auto nsamples = input.kdtree_get_point_count();
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
    delaunay_tetrahedrisation = new dt(
        boost::make_zip_iterator(boost::make_tuple( dtpoints.begin(), vinfos.begin() )),
        boost::make_zip_iterator(boost::make_tuple( dtpoints.end(), vinfos.end() ) ),
        &locking_datastructure );

    if( extended_bounding_box )
      {
        bbox.m_hsides *= 5.0;
        auto minp = bbox.get_min();
        auto maxp = bbox.get_max();
        delaunay_tetrahedrisation->insert( dt::Point( minp.x, minp.y, minp.z ) )->info() = null_dt_vertex_info;
        delaunay_tetrahedrisation->insert( dt::Point( maxp.x, minp.y, minp.z ) )->info() = null_dt_vertex_info;
        delaunay_tetrahedrisation->insert( dt::Point( minp.x, maxp.y, minp.z ) )->info() = null_dt_vertex_info;
        delaunay_tetrahedrisation->insert( dt::Point( maxp.x, maxp.y, minp.z ) )->info() = null_dt_vertex_info;
        delaunay_tetrahedrisation->insert( dt::Point( minp.x, minp.y, maxp.z ) )->info() = null_dt_vertex_info;
        delaunay_tetrahedrisation->insert( dt::Point( maxp.x, minp.y, maxp.z ) )->info() = null_dt_vertex_info;
        delaunay_tetrahedrisation->insert( dt::Point( minp.x, maxp.y, maxp.z ) )->info() = null_dt_vertex_info;
        delaunay_tetrahedrisation->insert( dt::Point( maxp.x, maxp.y, maxp.z ) )->info() = null_dt_vertex_info;
      }

    //release now the memory
    std::vector<dt::Point>{}.swap( dtpoints );
    std::vector<dt_vertex_info>{}.swap( vinfos );

    voronoi_balls.resize( delaunay_tetrahedrisation->number_of_finite_cells() );
    size_t voronoi_ball_index = 0;
    for( auto cit = delaunay_tetrahedrisation->finite_cells_begin(),
        end = delaunay_tetrahedrisation->finite_cells_end(); cit != end; ++ cit )
      {
        auto& info = voronoi_balls[ voronoi_ball_index ];
        auto voronoi_vertex = delaunay_tetrahedrisation->dual( cit );
        info.atom = vec4 {
          CGAL::to_double( voronoi_vertex.x() ),
          CGAL::to_double( voronoi_vertex.y() ),
          CGAL::to_double( voronoi_vertex.z() ),
          CGAL::to_double(CGAL::squared_distance( cit->vertex(0)->point(), voronoi_vertex ))
        };
        cit->info() = &info;
        ++voronoi_ball_index;
      }

    # pragma omp parallel for
    for( size_t i = 0; i < voronoi_ball_index; ++ i )
      {
        auto& info = voronoi_balls[ i ];
        if ( input.contain( vec3(info.atom) ) )
          info.status |= voronoi_ball::INSIDE_SHAPE;
      }
  }

  typedef CGAL::Epick rt_kernel;
  typedef CGAL::Regular_triangulation_euclidean_traits_3< rt_kernel > rt_traits;
  typedef CGAL::Triangulation_vertex_base_with_info_3< size_t, rt_traits > rt_vertex_base;
  typedef CGAL::Regular_triangulation_cell_base_3< rt_traits > rt_cell_base;
  typedef CGAL::Triangulation_data_structure_3< rt_vertex_base, rt_cell_base, CGAL::Parallel_tag > rt_datastructure;
  typedef CGAL::Regular_triangulation_3< rt_traits, rt_datastructure > rt;


  void powershape_structuration(
      median_skeleton& skeleton,
      std::vector< voronoi_ball >& voronoi_balls,
      const structurer::parameters& params )
  {
    size_t size = voronoi_balls.size();
    for( size_t i = 0; i < size; )
      {
        if( !(voronoi_balls[i].status & voronoi_ball::KEPT ))
          {
            if( i + 1 < size )
              {
                std::swap( voronoi_balls[i], voronoi_balls.back() );
              }
            voronoi_balls.pop_back();
            --size;
          }
        else ++i;
      }

    tbb::task_scheduler_init init;
    std::vector< rt::Weighted_point > wpoints( size );
    std::vector< size_t > vinfos( size );
# pragma omp declare reduction(minvballcenter: vec3: omp_out = min( omp_out, omp_in )) \
  initializer(omp_priv = vec3{REAL_MAX,REAL_MAX,REAL_MAX})
# pragma omp declare reduction(maxvballcenter: vec3: omp_out = max( omp_out, omp_in )) \
  initializer(omp_priv = vec3{-REAL_MAX,-REAL_MAX,-REAL_MAX})
    vec3 minp = vec3{REAL_MAX,REAL_MAX,REAL_MAX};
    vec3 maxp = vec3{-REAL_MAX,-REAL_MAX,-REAL_MAX};
    # pragma omp parallel for reduction(minvballcenter:minp) reduction(maxvballcenter:maxp)
    for( size_t i = 0; i < size; ++ i )
      {
        const auto& atom = voronoi_balls[ i ].atom;
        auto p = vec3( atom );
        wpoints[ i ] = rt::Weighted_point( rt::Bare_point( atom.x, atom.y, atom.z ), atom.w * atom.w );
        vinfos[ i ] = i;
        minp = min( minp, p );
        maxp = max( maxp, p );
      }

    rt::Lock_data_structure locking_datastructure(
        CGAL::Bbox_3(
            minp.x - 1e-6, minp.y - 1e-6, minp.z - 1e-6,
            maxp.x + 1e-6, maxp.y + 1e-6, maxp.z + 1e-6),
        50 );
    rt regular_tetrahedrization(
        boost::make_zip_iterator( boost::make_tuple( wpoints.begin(), vinfos.begin())),
        boost::make_zip_iterator( boost::make_tuple( wpoints.end()  , vinfos.end()  )),
        &locking_datastructure);

    if( params.m_build_faces )
      {
        auto nb_finite_facets = regular_tetrahedrization.number_of_finite_facets();
        // estimation of the number of faces: half of the finite facets
        skeleton.reserve_faces( nb_finite_facets >> 1 );
        // estimation of the number of links: 3 times the estimate of the number of faces
        skeleton.reserve_links( nb_finite_facets * 1.5 );

        median_skeleton::atom_index indices[3];
        for( auto fit = regular_tetrahedrization.finite_facets_begin(),
            fitend = regular_tetrahedrization.finite_facets_end(); fit != fitend;
            ++ fit )
          {
            int j = 0;
            for( int i = 0; i < 4; ++ i )
              {
                if( i != fit->second )
                  {
                    auto& info = voronoi_balls[ fit->first->vertex( i )->info() ];
                    if( info.status == voronoi_ball::ATOM )
                      {
                        indices[j] = info.idx;
                        ++j;
                      }
                  }
              }

            if( j == 3 )
              {
                if( !params.m_neighbors_should_intersect
                    || ( skeleton.get_atom_by_index( indices[0] ).intersect( skeleton.get_atom_by_index( indices[1] ) )
                      && skeleton.get_atom_by_index( indices[0] ).intersect( skeleton.get_atom_by_index( indices[2] ) )
                      && skeleton.get_atom_by_index( indices[1] ).intersect( skeleton.get_atom_by_index( indices[2] ) ) ) )
                  {
                    skeleton.add( indices[0], indices[1], indices[2] );
                  }
              }
          }
      }
    else
      {
        skeleton.reserve_links( regular_tetrahedrization.number_of_finite_edges() >> 1);
        for( auto eit = regular_tetrahedrization.finite_edges_begin(),
            eitend = regular_tetrahedrization.finite_edges_end(); eit != eitend;
            ++ eit )
          {
            auto& i1 = voronoi_balls[ eit->first->vertex( eit->second )->info() ];
            auto& i2 = voronoi_balls[ eit->first->vertex( eit->third  )->info() ];
            if( (i1.status == voronoi_ball::ATOM) && (i2.status == voronoi_ball::ATOM )
             && (!params.m_neighbors_should_intersect || skeleton.get_atom_by_index( i1.idx ).intersect( skeleton.get_atom_by_index( i2.idx ) ) ) )
              {
                skeleton.add( i1.idx, i2.idx );
              }
          }
      }
  }

  void voronoi_structuration(
      median_skeleton& skeleton,
      std::vector< voronoi_ball >& voronoi_balls,
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
                // not relevant vertex or not representing an atom
                // note: an atom has a status of 3 since it is inside the shape and kept for the structuration
                if( !info || info->status != voronoi_ball::ATOM )
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
                    for( size_t i = 2; i < nelements; ++ i )
                      {
                        if( vballs[i]->atom.intersect( vballs[0]->atom )
                         && vballs[0]->atom.intersect( vballs[i-1]->atom )
                         && vballs[i]->atom.intersect( vballs[i-1]->atom ))
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
    else
      {
        skeleton.reserve_links( delaunay_tetrahedrization.number_of_finite_facets() );
        for( auto ffit = delaunay_tetrahedrization.finite_facets_begin(),
            ffend = delaunay_tetrahedrization.finite_facets_end(); ffit != ffend;
            ++ ffit )
          {
            auto pi1 = ffit->first->info();
            auto pi2 = ffit->first->neighbor( ffit->second )->info();

            if( pi1 && pi2 &&
                (pi1->status == voronoi_ball::ATOM ) && (pi2->status == voronoi_ball::ATOM ) &&
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
    std::vector< voronoi_ball > voronoi_balls;
    dt* delaunay_tetrahedrisation;
    build_and_classify_voronoi_balls( input, output, voronoi_balls, delaunay_tetrahedrisation,
      // need an extended bounding box only to apply a powershape algorithm (gives more outside voronoi vertices)
      params.m_build_topology && (params.m_structurer_parameters.m_topology_method == structurer::parameters::POWERSHAPE) );

    const auto nb_balls = voronoi_balls.size();

    if( params.m_build_topology )
      {
        if( params.m_structurer_parameters.m_topology_method == structurer::parameters::VORONOI )
          {
            median_skeleton::atom_index atom_index = 0;
            # pragma omp parallel for
            for( size_t i = 0; i < nb_balls; ++ i )
              {
                auto& info = voronoi_balls[ i ];
                if( info.status & voronoi_ball::INSIDE_SHAPE )
                  {
                    info.atom.w = std::sqrt( info.atom.w );
                    info.status = voronoi_ball::ATOM;
                    # pragma omp critical
                    {
                      output.add( info.atom );
                      info.idx = atom_index;
                      ++atom_index;
                    }
                  }
              }
            voronoi_structuration( output, voronoi_balls, *delaunay_tetrahedrisation, params.m_structurer_parameters );
          }
        else if( params.m_structurer_parameters.m_topology_method == structurer::parameters::POWERSHAPE )
          {
            median_skeleton::atom_index atom_index = 0;
            # pragma omp parallel for
            for( size_t i = 0; i < nb_balls; ++ i )
              {
                auto& info = voronoi_balls[ i ];
                info.atom.w = std::sqrt( info.atom.w );
                if( info.status & voronoi_ball::INSIDE_SHAPE )
                  {
                    # pragma omp critical
                    {
                      output.add( info.atom );
                      info.idx = atom_index;
                      ++atom_index;
                    }
                  }
                info.status |= voronoi_ball::KEPT;
              }
            powershape_structuration( output, voronoi_balls, params.m_structurer_parameters );
          }
      }
    delete delaunay_tetrahedrisation;
  }


  void polar_ball_skeletonizer(
      graphics_origin::geometry::mesh_spatial_optimization& input,
      median_skeleton& output,
      const skeletonizer::parameters& params )
  {
    std::vector< voronoi_ball > voronoi_balls;
    dt* delaunay_tetrahedrisation;
    build_and_classify_voronoi_balls( input, output, voronoi_balls, delaunay_tetrahedrisation, true );

    const auto nb_balls = voronoi_balls.size();
    std::vector< dt::Cell_handle > incident_cells;
    for( auto fvit = delaunay_tetrahedrisation->finite_vertices_begin(),
        fvitend = delaunay_tetrahedrisation->finite_vertices_end(); fvit != fvitend;
        ++ fvit )
      {
        if( fvit->info() != null_dt_vertex_info )
          {

            delaunay_tetrahedrisation->incident_cells( fvit, std::back_inserter( incident_cells));

            voronoi_ball* in_pole = nullptr;
            voronoi_ball* out_pole = nullptr;
            for( auto& ch : incident_cells )
              {
                if( ch->info()->status & voronoi_ball::INSIDE_SHAPE )
                  {
                    if( !in_pole || in_pole->atom.w < ch->info()->atom.w )
                      in_pole = ch->info();
                  }
                else
                  {
                    if( !out_pole || out_pole->atom.w < ch->info()->atom.w )
                      out_pole = ch->info();
                  }
              }
            if( in_pole ) in_pole->status |= voronoi_ball::KEPT;
            if( out_pole ) out_pole->status |= voronoi_ball::KEPT;
            incident_cells.clear();
          }
      }



    if( params.m_build_topology && params.m_structurer_parameters.m_topology_method == structurer::parameters::POWERSHAPE )
      {
        median_skeleton::atom_index atom_index = 0;
        # pragma omp parallel for
        for( size_t i = 0; i < nb_balls; ++ i )
          {
            auto& info = voronoi_balls[ i ];
            if( info.status & voronoi_ball::KEPT )
              info.atom.w = std::sqrt( info.atom.w );
            if( info.status == voronoi_ball::ATOM )
              {
                # pragma omp critical
                {
                  output.add( info.atom );
                  info.idx = atom_index;
                  ++atom_index;
                }
              }
          }
        powershape_structuration( output, voronoi_balls, params.m_structurer_parameters );
      }
    else
      {
        median_skeleton::atom_index atom_index = 0;
        # pragma omp parallel for
        for( size_t i = 0; i < nb_balls; ++ i )
          {
            auto& info = voronoi_balls[ i ];
            if( info.status == voronoi_ball::ATOM )
              {
                # pragma omp critical
                {
                  auto& info = voronoi_balls[ i ];
                  info.atom.w = std::sqrt( info.atom.w );
                  output.add( info.atom );
                  info.idx = atom_index;
                  ++atom_index;
                }
              }
          }
      }
    delete delaunay_tetrahedrisation;

  }


END_MP_NAMESPACE
