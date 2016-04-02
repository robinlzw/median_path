/*  Created on: Mar 27, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "../median-path/skeletonization.h"

# include <graphics-origin/tools/log.h>

# define CGAL_LINKED_WITH_TBB
# include <tbb/task_scheduler_init.h>
# include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
# include <CGAL/Triangulation_vertex_base_with_info_2.h>
# include <CGAL/Triangulation_face_base_2.h>
# include <CGAL/Delaunay_triangulation_2.h>

BEGIN_MP_NAMESPACE

  typedef CGAL::Epick dt_kernel;
  typedef CGAL::Triangulation_vertex_base_with_info_2< median_skeleton::atom_index, dt_kernel > dt_vertex_base;
  typedef CGAL::Triangulation_face_base_2< dt_kernel > dt_face_base;
  typedef CGAL::Triangulation_data_structure_2< dt_vertex_base, dt_face_base > dt_datastructure;
  typedef CGAL::Delaunay_triangulation_2< dt_kernel, dt_datastructure > dt;

  void delaunay_reconstruction(
      median_skeleton& skeleton,
      graphics_origin::geometry::mesh_spatial_optimization& msp,
      std::vector< std::vector< uint32_t > >& vertex_to_atoms,
      const structurer::parameters& params )
  {
    const auto ntriangles = msp.get_number_of_triangles();
    # pragma omp parallel
    {
      std::vector< median_skeleton::atom_index > indices;
      median_skeleton::atom_index face[3];
      # pragma omp for
      for( uint32_t i = 0; i < ntriangles; ++ i )
        {
          auto fvit = msp.get_geometry().fv_begin( graphics_origin::geometry::mesh::FaceHandle(i) );
          for( int j = 0; j < 3; ++ j, ++ fvit )
            {
              for( auto& k : vertex_to_atoms[ fvit->idx() ] )
                {
                  indices.push_back( k );
                }
            }

          std::sort( indices.begin(), indices.end() );
          auto last = std::unique( indices.begin(), indices.end() );

          dt delaunay_triangulation;
          const auto& triangle = msp.get_triangle( i );
          const vec3 e1 = normalize( triangle.get_vertex(graphics_origin::geometry::triangle::V1) - triangle.get_vertex(graphics_origin::geometry::triangle::V0) );
          const vec3 e2 = cross( triangle.get_normal(), e1 );

          for( auto begin = indices.begin(); begin != last; ++ begin )
            {
              vec3 p = vec3{skeleton.get_atom_by_index( *begin ) };
              p -= dot( triangle.get_normal(), p - triangle.get_vertex(graphics_origin::geometry::triangle::V0) );
              delaunay_triangulation.insert( dt::Point( dot( p, e1 ), dot( p, e2 ) ) )->info() = *begin;
            }
          indices.resize( 0 );


          if( params.m_build_faces )
            {
              for( auto fit = delaunay_triangulation.finite_faces_begin(),
                  fitend = delaunay_triangulation.finite_faces_end(); fit != fitend;
                  ++fit )
                {
                  face[0] = fit->vertex(0)->info();
                  face[1] = fit->vertex(1)->info();
                  face[2] = fit->vertex(2)->info();

                  if( params.m_neighbors_should_intersect )
                    {
                      const auto& atom0 = skeleton.get_atom_by_index( face[0] );
                      const auto& atom1 = skeleton.get_atom_by_index( face[1] );
                      const auto& atom2 = skeleton.get_atom_by_index( face[2] );

                      if( !atom0.intersect( atom1 ) || !atom0.intersect( atom2 ) || !atom1.intersect( atom2 ) )
                        {
                          continue;
                        }
                    }

                  # pragma omp critical
                  skeleton.add( face[0], face[1], face[2] );
                }
            }
//          else
            {
              for( auto eit = delaunay_triangulation.finite_edges_begin(),
                  eitend = delaunay_triangulation.finite_edges_end(); eit != eitend;
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

                  if( params.m_neighbors_should_intersect &&
                      !skeleton.get_atom_by_index( face[0] ).intersect( skeleton.get_atom_by_index( face[1 ] ) ) )
                    continue;

                  # pragma omp critical
                  skeleton.add( face[0], face[1] );
                }
            }
        }
    }
  }
END_MP_NAMESPACE

