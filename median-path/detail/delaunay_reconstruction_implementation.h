# define CGAL_LINKED_WITH_TBB
# include <tbb/task_scheduler_init.h>
# include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
# include <CGAL/Triangulation_vertex_base_with_info_2.h>
# include <CGAL/Triangulation_face_base_2.h>
# include <CGAL/Delaunay_triangulation_2.h>

namespace median_path {

  namespace structurer2 {

    namespace {

      template< typename atomizer_type >
# ifdef MP_USE_CONCEPTS
        requires Atomizer<atomizer_type>()
# endif
      struct vertex_to_atoms_helper {
        typedef int const_iterator;

        vertex_to_atoms_helper(
            atomizer_type& atomizer,
            skeletonizable_shape& shape,
            median_skeleton& result )
        {
          static_assert(
              implementation_required<atomizer_type>::value,
              "please, provide an implementation of vertex_to_atoms_helper for this type of atomizer");
        }

        const_iterator begin( uint32_t vertex_index ) const;
        const_iterator end( uint32_t vertex_index ) const;
      };

      template<>
      struct vertex_to_atoms_helper<atomizer::shrinking_ball_vertex_constant_initial_radius> {

        typedef atomizer::shrinking_ball_vertex_constant_initial_radius atomizer_type;
        typedef atomizer_type::atom_to_sampling_type atom_to_sampling_type;
        typedef std::vector<median_skeleton::atom_index>::const_iterator const_iterator;

        vertex_to_atoms_helper(
            atomizer_type& atomizer,
            skeletonizable_shape& shape,
            median_skeleton& result )
          : storage( shape.n_vertices() * 2, median_skeleton::null_atom_index )
        {
          const auto natoms = result.get_number_of_atoms();
          auto& sampling_property = result.get_atom_property( atomizer.atom_to_sampling_property_index );
          for( median_skeleton::atom_index i = 0; i < natoms; ++ i )
            {
              auto& sampling = sampling_property.get<atom_to_sampling_type>( i );

              size_t j = sampling[0] << 1;
              if( storage[j] != median_skeleton::null_atom_index )
                ++j;
              storage[j] = i;

              j = sampling[1] << 1;
              if( storage[j] != median_skeleton::null_atom_index )
                ++j;
              storage[j] = i;
            }
        }

        const_iterator begin( uint32_t vertex_index ) const {
          return storage.begin() + (vertex_index<<1);
        }

        const_iterator end( uint32_t vertex_index ) const {
          return storage.begin() + ((vertex_index+1)<<1);
        }

        std::vector< median_skeleton::atom_index > storage;
      };
    }


    template< typename atomizer_type >
# ifdef MP_USE_CONCEPTS
        requires Atomizer<atomizer_type>()
# endif
    delaunay_reconstruction::delaunay_reconstruction(
        parameters_type const& parameters,
        skeletonizable_shape const& shape,
        atomizer_type& atomizer, median_skeleton& result )
    {
      static_assert(
          std::is_same<
            typename atomizer_type::sampling_type,
            atomizer::vertex_sampling >::value,
          "delaunay reconstruction can only be applied on a vertex sampling atomizer");

      typedef CGAL::Epick dt_kernel;
      typedef CGAL::Triangulation_vertex_base_with_info_2< median_skeleton::atom_index, dt_kernel > dt_vertex_base;
      typedef CGAL::Triangulation_face_base_2< dt_kernel > dt_face_base;
      typedef CGAL::Triangulation_data_structure_2< dt_vertex_base, dt_face_base > dt_datastructure;
      typedef CGAL::Delaunay_triangulation_2< dt_kernel, dt_datastructure > dt;


      /* We have T triangles, V vertices, E edges and N atoms.
       * In average, we have E ~ 3 V and F = 2 V.
       *
       * For this structuration, we need for each triangle t = {v0,v1,v2}, which
       * atoms that obtained by v0, v1 or v2 as samples during the atomization.
       * All we have, is the samples used by the atomization for each atom. We
       * now speak about solutions to get the needed information.
       *
       * The first solution is for each triangle, find among atoms those in
       * relationship with the triangle vertices. This means a linear complexity
       * for each triangle in the number of atoms, thus a total time complexity of
       * O( N x T ) = O( 2 V x T ) = O( V x T ) and space complexity of O( 1 ).
       * We do not need any synchronization to build such information.
       *
       * Another solution is to build a vector of size T, where each element is
       * the list of atoms related to a triangle. This represents a time
       * complexity of O( N ) and a spatial complexity of O( T ). In
       * case we use a parallelization to built such information, we would need
       * synchronization each time we add an atom to a triangle.
       *
       * A last solution is to build a vector of size V, where each element is
       * the list of atoms related to a vertex. Then, for each triangle, we look
       * in constant time in that vector to build the list of related atoms.
       * We have here a time complexity of O(N) and a spatial complexity of
       * O( V ). We need synchronization during the construction of the vector.
       *
       * The first solution is obviously too costly in time.
       * In case we use a shrinking ball or a power ball atomizer, we have
       * roughly two atoms per vertex, thus the second solution requires
       * 2 V x 3 x 2 = 12 V atom indices and the third solution requires
       *   V     x 2 =  2 V atom indices. For a voronoi atomizer, we have
       * 4 atoms per vertex, thus the second solution requires
       * 2 V x 3 x 4 = 24 V atom indices and the third solution requires
       *   V     x 4 =  4 V atom indices. Also, the third solution should
       * be faster, because there are less things to do in an iteration.
       * For such reasons, I chose the third solution.
       *
       * Now, should we really parallelize the construction of the vector?
       * We will spend most, if not all, the time waiting for a critical
       * section. Thus, no parallelization here. */

      vertex_to_atoms_helper<atomizer_type> helper( atomizer, shape, result );
      graphics_origin::geometry::mesh_point_converter<vec3> point_converter;
      graphics_origin::geometry::mesh_normal_converter<vec3> normal_converter;
      const auto ntriangles = shape.n_faces();
      # pragma omp parallel
      {
        std::vector< median_skeleton::atom_index > indices;
        median_skeleton::atom_index face[3];

        # pragma omp for
        for( uint32_t i = 0; i < ntriangles; ++ i )
          {
            auto fvit = shape.cfv_begin( skeletonizable_shape::FaceHandle(i) );

            vec3 p0 = point_converter( shape.point( fvit ) );
            for( auto it = helper.begin( fvit->idx() ), end = helper.end( fvit->idx() );
                it != end; ++ it )
              indices.push_back( *it );
            ++fvit;

            vec3 e01 = normalize( point_converter( shape.point( fvit ) ) - p0 );
            for( auto it = helper.begin( fvit->idx() ), end = helper.end( fvit->idx() );
                it != end; ++ it )
              indices.push_back( *it );
            ++fvit;

            for( auto it = helper.begin( fvit->idx() ), end = helper.end( fvit->idx() );
                it != end; ++ it )
              indices.push_back( *it );

            dt triangulation;
            std::sort( indices.begin(), indices.end() );
            auto last = std::unique( indices.begin(), indices.end() );

            vec3 normal = normal_converter( shape.normal( skeletonizable_shape::FaceHandle(i) ) );
            vec3 other = cross( normal, e01 );

            // basis: (p0; e01, other, normal )

            for( auto begin = indices.begin(); begin != last; ++ begin )
              {
                vec3 p = vec3{ result.get_atom_by_index( *begin ) };
                p -= dot( normal, p - p0 );
                triangulation.insert( dt::Point( dot( p, e01 ), dot( p, other ) ) )->info() = *begin;
              }
            indices.resize( 0 ); // to be reused at the next iteration

            if( parameters.build_faces )
              {
                for( auto fit = triangulation.finite_faces_begin(),
                    fitend = triangulation.finite_faces_end(); fit != fitend;
                    ++fit )
                  {
                    face[0] = fit->vertex(0)->info();
                    face[1] = fit->vertex(1)->info();
                    face[2] = fit->vertex(2)->info();

                    if( parameters.neighbors_must_intersect )
                      {
                        const auto& atom0 = result.get_atom_by_index( face[0] );
                        const auto& atom1 = result.get_atom_by_index( face[1] );
                        const auto& atom2 = result.get_atom_by_index( face[2] );

                        if( !atom0.intersect( atom1 ) || !atom0.intersect( atom2 ) || !atom1.intersect( atom2 ) )
                          {
                            continue;
                          }
                      }

                    # pragma omp critical
                    result.add( face[0], face[1], face[2] );
                  }
              }

            for( auto eit = triangulation.finite_edges_begin(),
                eitend = triangulation.finite_edges_end(); eit != eitend;
                ++ eit )
              {
                int k = 0;
                for( int j = 0; j < 3; ++ j )
                  {
                    if( j != eit->second )
                      {
                        face[k] = eit->first->vertex( j )->info();
                        ++k;
                      }
                  }

                if( parameters.neighbors_must_intersect &&
                    !result.get_atom_by_index( face[0] ).intersect( result.get_atom_by_index( face[1 ] ) ) )
                  continue;

                # pragma omp critical
                result.add( face[0], face[1] );
              }
          } // end of parallel for
      } // end of parallel region
    }

  } // end of structurer namespace

} // end of median_path namespace
