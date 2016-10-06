# include "../median-path/structuration.h"

# define CGAL_LINKED_WITH_TBB
# include <tbb/task_scheduler_init.h>
# include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
# include <CGAL/Triangulation_vertex_base_with_info_3.h>
# include <CGAL/Regular_triangulation_euclidean_traits_3.h>
# include <CGAL/Regular_triangulation_3.h>

# include <CGAL/Fixed_alpha_shape_3.h>
# include <CGAL/Fixed_alpha_shape_vertex_base_3.h>
# include <CGAL/Fixed_alpha_shape_cell_base_3.h>

namespace median_path {

  template< typename Info_, typename GT, typename Vb = CGAL::Fixed_alpha_shape_vertex_base_3<GT> >
  class Fixed_alpha_shape_vertex_base_with_info_3 : public Vb  {
    Info_ _info;
  public:
    typedef typename Vb::Cell_handle                   Cell_handle;
    typedef typename Vb::Point                         Point;
    typedef Info_                                      Info;

    template < typename TDS2 >
    struct Rebind_TDS {
      typedef typename Vb::template Rebind_TDS<TDS2>::Other          Vb2;
      typedef Fixed_alpha_shape_vertex_base_with_info_3<Info, GT, Vb2>   Other;
    };

    Fixed_alpha_shape_vertex_base_with_info_3()
      : Vb() {}

    Fixed_alpha_shape_vertex_base_with_info_3(const Point & p)
      : Vb(p) {}

    Fixed_alpha_shape_vertex_base_with_info_3(const Point & p, Cell_handle c)
      : Vb(p, c) {}

    Fixed_alpha_shape_vertex_base_with_info_3(Cell_handle c)
      : Vb(c) {}

    const Info& info() const { return _info; }
    Info&       info()       { return _info; }
  };

  typedef CGAL::Epick rt_kernel;
  typedef CGAL::Regular_triangulation_euclidean_traits_3< rt_kernel > rt_traits;
  typedef Fixed_alpha_shape_vertex_base_with_info_3< median_skeleton::atom_index, rt_traits > rt_vertex_base;
  typedef CGAL::Fixed_alpha_shape_cell_base_3< rt_traits > rt_cell_base;
  typedef CGAL::Triangulation_data_structure_3< rt_vertex_base, rt_cell_base, CGAL::Parallel_tag > rt_datastructure;
  typedef CGAL::Regular_triangulation_3< rt_traits, rt_datastructure > rt;
  typedef CGAL::Fixed_alpha_shape_3< rt > fixed_alpha_shape;

  namespace structurer2 {

    weighted_zero_shape::weighted_zero_shape( parameters_type const& parameters, median_skeleton& result )
    {
      tbb::task_scheduler_init init;

      // compute and store input of the regular tetrahedrization (RT)
      const auto natoms = result.get_number_of_atoms();
      std::vector< rt::Weighted_point > wpoints( natoms );
      std::vector< median_skeleton::atom_index > vinfos( natoms );
      # pragma omp parallel for schedule(static)
      for( median_skeleton::atom_index i = 0; i < natoms; ++ i )
        {
          const auto& atom = result.get_atom_by_index( i );
          wpoints[ i ] = rt::Weighted_point( rt::Bare_point( atom.x, atom.y, atom.z ), atom.w * atom.w );
          vinfos[ i ] = i;
        }

      // compute RT
      auto bbox = result.compute_centers_bounding_box();
      bbox.hsides += 1e-6;
      rt::Lock_data_structure locking_datastructure(
          CGAL::Bbox_3(
              bbox.center.x - bbox.hsides.x, bbox.center.y - bbox.hsides.y, bbox.center.z - bbox.hsides.z,
              bbox.center.x + bbox.hsides.x, bbox.center.y + bbox.hsides.y, bbox.center.z + bbox.hsides.z ),
          parameters.grid_subdivisions);
      rt regular_tetrahedrization(
          boost::make_zip_iterator(boost::make_tuple( wpoints.begin(), vinfos.begin() )),
          boost::make_zip_iterator(boost::make_tuple( wpoints.end()  , vinfos.end()   )),
          &locking_datastructure);

      // explicitly release RT's input, because it is not needed anymore
      std::vector< rt::Weighted_point >{}.swap( wpoints );
      std::vector< median_skeleton::atom_index >{}.swap( vinfos );

      // compute the weighted zero shape (W0S)
      fixed_alpha_shape alpha_shape( regular_tetrahedrization, 0 );

      // To avoid to resize too often the link buffers, we estimate the number of links
      // from the number of finite edges. The final number of edges will be smaller or
      // equal to that estimation.
      result.reserve_links( alpha_shape.number_of_finite_edges() );

      if( parameters.build_faces )
        {
          // To avoid to resize too often the face buffers, we estimate the
          // number of faces from the number of finite facets. The final number
          // of faces will be smaller or equal to that estimation.
          result.reserve_faces( alpha_shape.number_of_finite_facets() );

          // Most of the finite facets will be included in the skeleton. Thus,
          // if we process facets in parallel, threads will (almost) always
          // request to enter in a critical section to do their job. This is why
          // this step is not done in parallel (note: the VMV2016 benchmark
          // confirmed that this step should be done sequentially).
          median_skeleton::atom_index indices[3];
          for( auto fit = alpha_shape.facets_begin(), fitend = alpha_shape.facets_end();
              fit != fitend; ++ fit )
            {
              auto type = alpha_shape.classify( *fit );
              if( type == fixed_alpha_shape::REGULAR || type == fixed_alpha_shape::SINGULAR )
                {
                  int j = 0;
                  for( int i = 0; i < 4; ++ i )
                    {
                      if( i != fit->second )
                        {
                          indices[j] = fit->first->vertex( i )->info();
                          ++j;
                        }
                    }
                  result.add( indices[0], indices[1], indices[2] );
                }
            }
        }

      // Even if we build faces, some edges are not already in the skeleton
      // since they are not part of any triangle. Thus, this step should be
      // executed in any case. If we want to do this step in parallel, most
      // of the work will be to check if a link already exist in the skeleton.
      // This verification is already done in the add( atom_index, atom_index )
      // method to avoid duplicate links. Thus, in parallel we will need to
      // perform this verification two times: one time to decide whether to
      // enter in a critical section and one time in the add() method. Thus,
      // a sequential processing is a better idea here.
      for( auto eit = alpha_shape.edges_begin(), eitend = alpha_shape.edges_end();
          eit != eitend; ++ eit )
        {
          auto type = alpha_shape.classify( *eit );
          if( type == fixed_alpha_shape::REGULAR || type == fixed_alpha_shape::SINGULAR )
            {
              result.add( eit->first->vertex( eit->second )->info(), eit->first->vertex( eit->third )->info() );
            }
        }
    }
  }
}

