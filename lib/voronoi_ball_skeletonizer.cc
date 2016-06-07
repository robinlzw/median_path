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

  ///fixme: be carefull with the intersection of squared balls!

  void
  delaunay_reconstruction(
    median_skeleton& skeleton,
    graphics_origin::geometry::mesh_spatial_optimization& msp,
    std::vector< std::vector< uint32_t > >& vertex_to_atoms,
    const structurer::parameters& params );

  // index of the mesh vertex corresponding to a DT vertex
  typedef uint32_t dt_vertex_info;
  static const dt_vertex_info null_dt_vertex_info = uint32_t( -1 );

  typedef CGAL::Epeck dt_kernel;
  struct voronoi_ball
  {
    /**
     * kept  inside  valid  value  meaning
     *  0      0       0      0    invalid
     *  ?      ?       1      ?    valid ball
     *  0      0       1      1    outside voronoi ball unused for structuration
     *  1      0       1      5    outside voronoi ball to keep for structuration (outside pole)
     *  0      1       1      3    inside voronoi ball unused for structuration (not an inside pole)
     *  1      1       1      7    inside voronoi ball to keep for structuration (atom)
     */
    enum
    {
      INVALID = 0, VALID = 1, INSIDE_SHAPE = 2, KEPT = 4, ATOM = 7
    };

    voronoi_ball( ) :
        ball {}, idx{ 0 }, status{ INVALID }
    {}

    voronoi_ball(
      const voronoi_ball& other ) :
        ball{ other.ball }, idx{ other.idx }, status{ other.status }
    {}

    voronoi_ball&
    operator=(
      const voronoi_ball& other )
    {
      ball = other.ball;
      idx = other.idx;
      status = other.status;
      return *this;
    }
    median_skeleton::atom ball;
    median_skeleton::atom_index idx;
    uint8_t status;
  };

  typedef CGAL::Triangulation_vertex_base_with_info_3< dt_vertex_info, dt_kernel > dt_vertex_base;
  typedef CGAL::Triangulation_cell_base_with_info_3< voronoi_ball, dt_kernel > dt_cell_base;
  typedef CGAL::Triangulation_data_structure_3< dt_vertex_base, dt_cell_base, CGAL::Parallel_tag > dt_datastructure;
  typedef CGAL::Delaunay_triangulation_3< dt_kernel, dt_datastructure > dt;

  static
  dt*
  build_and_classify_voronoi_balls(
    graphics_origin::geometry::mesh_spatial_optimization& input,
    const skeletonizer::voronoi_and_polar_balls_parameters& params,
    bool extended_bounding_box )
  {
    // Initialize a TBB scheduler for the DT construction.
    // It will automatically set the maximum number of threads to use.
    tbb::task_scheduler_init init;

    // Now, build the input of the DT construction.
    // Since this data is useless after the construction, we can release it
    // after the construction, to decrease the memory consumption.
    const auto nsamples = input.kdtree_get_point_count( );
    dt::Point* dtpoints = new dt::Point[ nsamples ];
    dt_vertex_info* vinfos = new dt_vertex_info[ nsamples ];
    # pragma omp parallel for schedule(static)
    for( uint32_t i = 0; i < nsamples; ++i )
      {
        auto p = input.get_point( i );
        dtpoints[i] = dt::Point( p[0], p[1], p[2] );
        vinfos[i] = i;
      }

    // Parallel DT construction requires a bounding box, to divide the
    // work into sub-regions.
    graphics_origin::geometry::aabox bbox = input.get_bounding_box( );
    bbox.m_hsides += 1e-6;
    dt::Lock_data_structure locking_datastructure(
      CGAL::Bbox_3( bbox.m_center.x - bbox.m_hsides.x,
        bbox.m_center.y - bbox.m_hsides.y, bbox.m_center.z - bbox.m_hsides.z,
        bbox.m_center.x + bbox.m_hsides.x, bbox.m_center.y + bbox.m_hsides.y,
        bbox.m_center.z + bbox.m_hsides.z ), params.m_dt_bounding_box_subdivisions );
    dt* delaunay_tetrahedrisation = new dt(
      boost::make_zip_iterator(
        boost::make_tuple( dtpoints, vinfos ) ),
      boost::make_zip_iterator(
        boost::make_tuple( dtpoints + nsamples, vinfos + nsamples ) ),
      &locking_datastructure );

    // Release now the memory of DT construction input.
    delete[] dtpoints;
    delete[] vinfos;

    // For the Power Shape structuration, it is better to extend the bounding
    // box and insert its vertices in the DT, as the produced Voronoi vertices
    // will constrain more the regular triangulation to improve the skeletal
    // structure approximation. This is done after the construction of the DT
    // of the shape samples, since I noticed it is much more faster to do it
    // in that order.
    if( extended_bounding_box )
      {
        bbox.m_hsides *= real(0.5) * params.m_bounding_box_scale_factor;
        auto minp = bbox.get_min( );
        auto maxp = bbox.get_max( );
        delaunay_tetrahedrisation->insert( dt::Point( minp.x, minp.y, minp.z ) )->info( ) = null_dt_vertex_info;
        delaunay_tetrahedrisation->insert( dt::Point( maxp.x, minp.y, minp.z ) )->info( ) = null_dt_vertex_info;
        delaunay_tetrahedrisation->insert( dt::Point( minp.x, maxp.y, minp.z ) )->info( ) = null_dt_vertex_info;
        delaunay_tetrahedrisation->insert( dt::Point( maxp.x, maxp.y, minp.z ) )->info( ) = null_dt_vertex_info;
        delaunay_tetrahedrisation->insert( dt::Point( minp.x, minp.y, maxp.z ) )->info( ) = null_dt_vertex_info;
        delaunay_tetrahedrisation->insert( dt::Point( maxp.x, minp.y, maxp.z ) )->info( ) = null_dt_vertex_info;
        delaunay_tetrahedrisation->insert( dt::Point( minp.x, maxp.y, maxp.z ) )->info( ) = null_dt_vertex_info;
        delaunay_tetrahedrisation->insert( dt::Point( maxp.x, maxp.y, maxp.z ) )->info( ) = null_dt_vertex_info;
      }

    // The remaining step, necessary for both the Voronoi and the polar balls method
    // is to build and classify the Voronoi balls.
    input.build_bvh( ); // this is necessary for contain()
    input.build_kdtree( ); // this is necessary for contain()
    # pragma omp parallel
    # pragma omp single
    for( auto cit = delaunay_tetrahedrisation->finite_cells_begin( ), end =
        delaunay_tetrahedrisation->finite_cells_end( ); cit != end; ++cit )
      {
        # pragma omp task firstprivate(cit)
        {
          auto& info = cit->info();
          auto voronoi_vertex = delaunay_tetrahedrisation->dual( cit );

          // this ball is valid
          info.status |= voronoi_ball::VALID;
          // here is its geometry
          info.ball = vec4{
            CGAL::to_double( voronoi_vertex.x( ) ),
            CGAL::to_double( voronoi_vertex.y( ) ),
            CGAL::to_double( voronoi_vertex.z( ) ),
            CGAL::to_double( CGAL::squared_distance( cit->vertex( 0 )->point( ), voronoi_vertex ) ) };
          // check if it is inside the shape
          if(// if one vertex is a bounding box vertex, we know the ball is outside
               cit->vertex( 0 )->info( ) != null_dt_vertex_info
            && cit->vertex( 1 )->info( ) != null_dt_vertex_info
            && cit->vertex( 2 )->info( ) != null_dt_vertex_info
            && cit->vertex( 3 )->info( ) != null_dt_vertex_info
            && input.contain( vec3{ info.ball } ) )
            info.status |= voronoi_ball::INSIDE_SHAPE;
        }
      }

    return delaunay_tetrahedrisation;
  }

  struct rt_vertex_info {
    rt_vertex_info() :
      ptr{ nullptr }
    {}

    rt_vertex_info( voronoi_ball* vb )
    {
      ptr = vb;
    }

    rt_vertex_info( const rt_vertex_info& other ) :
      ptr{ other.ptr }
    {}

    rt_vertex_info&
    operator=( const rt_vertex_info& other )
    {
      ptr = other.ptr;
      return *this;
    }

    operator bool() const
    {
      return ptr;
    }

    voronoi_ball* operator->()
    {
      return ptr;
    }

    voronoi_ball* ptr;
  };

  typedef CGAL::Epick rt_kernel;
  typedef CGAL::Regular_triangulation_euclidean_traits_3< rt_kernel > rt_traits;
  typedef CGAL::Triangulation_vertex_base_with_info_3< rt_vertex_info, rt_traits > rt_vertex_base;
  typedef CGAL::Regular_triangulation_cell_base_3< rt_traits > rt_cell_base;
  typedef CGAL::Triangulation_data_structure_3< rt_vertex_base, rt_cell_base, CGAL::Parallel_tag > rt_datastructure;
  typedef CGAL::Regular_triangulation_3< rt_traits, rt_datastructure > rt;

  void
  powershape_structuration(
    median_skeleton& skeleton,
    dt* delaunay_tetrahedrisation,
    const structurer::parameters& params )
  {
    // Initialize a TBB scheduler for the DT construction.
    // It will automatically set the maximum number of threads to use.
    tbb::task_scheduler_init init;

    // Now, build the input of the RT construction.
    // Since this data is useless after the construction, we can release it
    // after the construction, to decrease the memory consumption.
    const size_t nb_finite_cells = delaunay_tetrahedrisation->number_of_finite_cells();
    rt::Weighted_point* wpoints = new rt::Weighted_point[ nb_finite_cells ];
    rt_vertex_info*  vinfos = new rt_vertex_info[ nb_finite_cells ];
    vec3 minp = vec3{ REAL_MAX, REAL_MAX, REAL_MAX };
    vec3 maxp = vec3{-REAL_MAX,-REAL_MAX,-REAL_MAX };
    size_t size = 0;
    for( auto cit = delaunay_tetrahedrisation->finite_cells_begin( ), end =
        delaunay_tetrahedrisation->finite_cells_end( ); cit != end; ++cit )
      {
        auto& info = cit->info( );
        if( info.status & voronoi_ball::KEPT )
          {
            vec3 p = vec3{ info.ball };
            minp = min( p, minp );
            maxp = max( p, maxp );
            wpoints[ size ] = rt::Weighted_point( rt::Bare_point( p.x, p.y, p.z ), info.ball.w * info.ball.w );
            vinfos[ size ] = &info;
            ++size;
          }
      }

    // Parallel RT construction requires a bounding box, to divide the
    // work into sub-regions.
    rt::Lock_data_structure locking_datastructure(
      CGAL::Bbox_3(
        minp.x - 1e-6, minp.y - 1e-6, minp.z - 1e-6,
        maxp.x + 1e-6, maxp.y + 1e-6, maxp.z + 1e-6 ),
        50 ); ///fixme: parameter
    rt regular_tetrahedrization(
      boost::make_zip_iterator(
        boost::make_tuple( wpoints, vinfos ) ),
      boost::make_zip_iterator(
        boost::make_tuple( wpoints + size, vinfos + size ) ),
      &locking_datastructure );

    // Release now the memory of RT construction input.
    delete[] wpoints;
    delete[] vinfos;

    auto nb_finite_facets = regular_tetrahedrization.number_of_finite_facets( );
    // estimation of the number of links: 3 times the estimate of the number of faces
    skeleton.reserve_links( nb_finite_facets * 1.5 );
    if( params.m_build_faces )
      {
        // estimation of the number of faces: half of the finite facets
        skeleton.reserve_faces( nb_finite_facets >> 1 );
        # pragma omp parallel
        {
          median_skeleton::atom_index indices[3];
          # pragma omp single
          for( auto fit = regular_tetrahedrization.finite_facets_begin( ),
              fitend = regular_tetrahedrization.finite_facets_end( );
              fit != fitend; ++fit )
            {
              # pragma omp task firstprivate(fit)
              {
                int j = 0;
                for( int i = 0; i < 4; ++i )
                  {
                    if( i != fit->second )
                      {
                        auto& info = fit->first->vertex( i )->info( );
                        if( info->status == voronoi_ball::ATOM )
                          {
                            indices[j] = info->idx;
                            ++j;
                          }
                      }
                  }

                if( j == 3 )
                  {
                    if( !params.m_neighbors_should_intersect
                      || (skeleton.get_atom_by_index( indices[0] ).intersect( skeleton.get_atom_by_index( indices[1] ) )
                       && skeleton.get_atom_by_index( indices[0] ).intersect( skeleton.get_atom_by_index( indices[2] ) )
                       && skeleton.get_atom_by_index( indices[1] ).intersect( skeleton.get_atom_by_index( indices[2] ) )) )
                      {
                        # pragma omp critical
                        skeleton.add( indices[0], indices[1], indices[2] );
                      }
                  }
              }
            }
        }
      }
    // This section should always be executed in order to get links not shared by any triangle
    # pragma omp parallel
    # pragma omp single
    for( auto eit = regular_tetrahedrization.finite_edges_begin( ), eitend =
        regular_tetrahedrization.finite_edges_end( ); eit != eitend; ++eit )
      {
        # pragma omp task firstprivate(eit)
        {
          auto& i1 = eit->first->vertex( eit->second )->info( );
          auto& i2 = eit->first->vertex( eit->third )->info( );
          if(    (i1->status == voronoi_ball::ATOM)
              && (i2->status == voronoi_ball::ATOM)
              && (!params.m_neighbors_should_intersect
                  || skeleton.get_atom_by_index( i1->idx ).intersect(skeleton.get_atom_by_index( i2->idx ) )) )
            {
              #pragma omp critical
              skeleton.add( i1->idx, i2->idx );
            }
        }
      }
  }

  void
  voronoi_structuration(
    median_skeleton& skeleton,
    dt* delaunay_tetrahedrization,
    const structurer::parameters& params )
  {
    skeleton.reserve_links( delaunay_tetrahedrization->number_of_finite_facets( ) );

    if( params.m_build_faces )
      {
        skeleton.reserve_faces( delaunay_tetrahedrization->number_of_finite_edges( ) );

        for( auto feit = delaunay_tetrahedrization->finite_edges_begin( ),
            feend = delaunay_tetrahedrization->finite_edges_end( );
            feit != feend; ++feit )
          {
            auto circulator = delaunay_tetrahedrization->incident_cells( *feit );
            std::vector< voronoi_ball* > vballs;
            auto begin = circulator;
            bool ok = true;
            do
              {
                auto info = circulator->info( );
                // not relevant vertex or not representing an atom
                if( info.status != voronoi_ball::ATOM )
                  {
                    ok = false;
                    break;
                  }
                vballs.push_back( &info );
                ++circulator;
              }
            while( circulator != begin );

            size_t nelements = vballs.size( );
            if( ok && nelements > 2 )
              {
                if( params.m_neighbors_should_intersect )
                  {
                    for( size_t i = 2; i < nelements; ++i )
                      {
                        if( vballs[i]->ball.intersect( vballs[0]->ball )
                            && vballs[0]->ball.intersect( vballs[i - 1]->ball )
                            && vballs[i]->ball.intersect(
                              vballs[i - 1]->ball ) )
                          {
                            skeleton.add( vballs[0]->idx, vballs[i - 1]->idx,
                              vballs[i]->idx );
                          }
                      }
                  }
                else
                  {

                    for( size_t i = 2; i < nelements; ++i )
                      {
                        skeleton.add( vballs[0]->idx, vballs[i - 1]->idx,
                          vballs[i]->idx );
                      }
                  }
              }
          }
      }
//    else
      {
        for( auto ffit = delaunay_tetrahedrization->finite_facets_begin( ),
            ffend = delaunay_tetrahedrization->finite_facets_end( );
            ffit != ffend; ++ffit )
          {
            auto pi1 = ffit->first->info( );
            auto pi2 = ffit->first->neighbor( ffit->second )->info( );

            if( (pi1.status == voronoi_ball::ATOM)
             && (pi2.status == voronoi_ball::ATOM)
             && (!params.m_neighbors_should_intersect
                || pi1.ball.intersect( pi2.ball )) )
              skeleton.add( pi1.idx, pi2.idx );
          }
      }
  }

  void
  voronoi_ball_skeletonizer(
    graphics_origin::geometry::mesh_spatial_optimization& input,
    median_skeleton& output, const skeletonizer::parameters& params )
  {
    bool keep_outside = params.m_build_topology && params.m_structurer_parameters.m_topology_method == structurer::parameters::POWERSHAPE;

    dt* delaunay_tetrahedrisation = build_and_classify_voronoi_balls(
      input,
      params.m_voronoi_ball,
      keep_outside );

    // add atoms to the skeleton, mark balls for structuration, compute the radius of balls necessary for structurations
    median_skeleton::atom_index atom_index = 0;
    for( auto cit = delaunay_tetrahedrisation->finite_cells_begin(), citend = delaunay_tetrahedrisation->finite_cells_end();
        cit != citend; ++ cit )
      {
        auto& info = cit->info();
        if( info.status & voronoi_ball::VALID )
          {
            if( info.status & voronoi_ball::INSIDE_SHAPE )
              {
                info.ball.w = std::sqrt( info.ball.w );
                output.add( info.ball );
                info.status |= voronoi_ball::KEPT;
                info.idx = atom_index;
                ++atom_index;
              }
            else if( keep_outside )
              {
                info.status |= voronoi_ball::KEPT;
                info.ball.w = std::sqrt( info.ball.w );
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
            powershape_structuration( output, delaunay_tetrahedrisation, params.m_structurer_parameters );
          }
        else if( params.m_structurer_parameters.m_topology_method == structurer::parameters::DELAUNAY_RECONSTRUCTION )
          {
            std::vector< std::vector< median_skeleton::atom_index > > vertex_to_atoms(
              input.kdtree_get_point_count( ) );
            for( auto cit = delaunay_tetrahedrisation->finite_cells_begin( ),
                end = delaunay_tetrahedrisation->finite_cells_end( );
                cit != end; ++cit )
              {
                auto& info = cit->info( );
                if( info.status == voronoi_ball::ATOM )
                  {
                    vertex_to_atoms[cit->vertex( 0 )->info( )].push_back( info.idx );
                    vertex_to_atoms[cit->vertex( 1 )->info( )].push_back( info.idx );
                    vertex_to_atoms[cit->vertex( 2 )->info( )].push_back( info.idx );
                    vertex_to_atoms[cit->vertex( 3 )->info( )].push_back( info.idx );
                  }
              }

            delaunay_reconstruction( output, input, vertex_to_atoms, params.m_structurer_parameters );
          }
      }
    delete delaunay_tetrahedrisation;
  }

  void
  polar_ball_skeletonizer(
    graphics_origin::geometry::mesh_spatial_optimization& input,
    median_skeleton& output, const skeletonizer::parameters& params )
  {
    dt* delaunay_tetrahedrisation = build_and_classify_voronoi_balls( input, params.m_voronoi_ball, true );

    // Identify poles and mark balls for structuration.
    bool keep_outside = params.m_build_topology && params.m_structurer_parameters.m_topology_method == structurer::parameters::POWERSHAPE;
    # pragma omp parallel
    {
      // We can reuse this vector for this thread to avoid reallocations.
      std::vector< dt::Cell_handle > incident_cells;

      # pragma omp single
      for( auto fvit = delaunay_tetrahedrisation->finite_vertices_begin( ),
          fvitend = delaunay_tetrahedrisation->finite_vertices_end(); fvit != fvitend;
          ++ fvit )
        {
          if( fvit->info( ) != null_dt_vertex_info )
            {
              # pragma omp task firstprivate(fvit)
              {
                delaunay_tetrahedrisation->incident_cells(
                  fvit,
                  std::back_inserter( incident_cells ) );

                voronoi_ball* in_pole = nullptr;
                voronoi_ball* out_pole = nullptr;
                for( auto& ch : incident_cells )
                  {
                    if( ch->info( ).status & voronoi_ball::INSIDE_SHAPE )
                      {
                        if( !in_pole || in_pole->ball.w < ch->info( ).ball.w )
                          in_pole = &ch->info( );
                      }
                    else if( keep_outside )
                      {
                        if( !out_pole || out_pole->ball.w < ch->info( ).ball.w )
                          out_pole = &ch->info( );
                      }
                  }
                if( in_pole )
                  in_pole->status |= voronoi_ball::KEPT;
                if( out_pole )
                  out_pole->status |= voronoi_ball::KEPT;
                incident_cells.clear( );
              }
            }
        }
    }

    // Add atoms to skeleton.
    median_skeleton::atom_index atom_index = 0;
    for( auto cit = delaunay_tetrahedrisation->finite_cells_begin(), citend = delaunay_tetrahedrisation->finite_cells_end();
        cit != citend; ++ cit )
      {
        auto& info = cit->info();
        if( info.status & voronoi_ball::VALID )
          {
            if( info.status & voronoi_ball::INSIDE_SHAPE )
              {
                output.add( vec3{info.ball}, std::sqrt( info.ball.w ) );
                info.idx = atom_index;
                ++atom_index;
              }
          }
      }

    // Power Shape structuration and Delaunay reconstruction are done here
    // since we need the Delaunay triangulation for those methods.
    if( params.m_build_topology )
      {
        if( params.m_structurer_parameters.m_topology_method == structurer::parameters::POWERSHAPE )
          {
            powershape_structuration( output, delaunay_tetrahedrisation, params.m_structurer_parameters );
          }
        else if( params.m_structurer_parameters.m_topology_method == structurer::parameters::DELAUNAY_RECONSTRUCTION )
          {
            std::vector< std::vector< median_skeleton::atom_index > > vertex_to_atoms(
              input.kdtree_get_point_count( ) );
            for( auto cit = delaunay_tetrahedrisation->finite_cells_begin( ),
                end = delaunay_tetrahedrisation->finite_cells_end( );
                cit != end; ++cit )
              {
                auto& info = cit->info( );
                if( info.status == voronoi_ball::ATOM )
                  {
                    vertex_to_atoms[cit->vertex( 0 )->info( )].push_back( info.idx );
                    vertex_to_atoms[cit->vertex( 1 )->info( )].push_back( info.idx );
                    vertex_to_atoms[cit->vertex( 2 )->info( )].push_back( info.idx );
                    vertex_to_atoms[cit->vertex( 3 )->info( )].push_back( info.idx );
                  }
              }

            delaunay_reconstruction( output, input, vertex_to_atoms, params.m_structurer_parameters );
          }
      }
    delete delaunay_tetrahedrisation;
  }

END_MP_NAMESPACE
