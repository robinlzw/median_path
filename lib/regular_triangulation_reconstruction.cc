/*  Created on: Mar 21, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "../median-path/skeletonization.h"

# include <graphics-origin/tools/log.h>

# define CGAL_LINKED_WITH_TBB
# include <tbb/task_scheduler_init.h>
# include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
# include <CGAL/Triangulation_vertex_base_with_info_3.h>
# include <CGAL/Regular_triangulation_euclidean_traits_3.h>
# include <CGAL/Regular_triangulation_3.h>
# include <CGAL/Simple_cartesian.h>

# include <CGAL/Fixed_alpha_shape_3.h>
# include <CGAL/Fixed_alpha_shape_vertex_base_3.h>
# include <CGAL/Fixed_alpha_shape_cell_base_3.h>

BEGIN_MP_NAMESPACE
  template < typename Info_, typename GT,
             typename Vb = CGAL::Fixed_alpha_shape_vertex_base_3<GT> >
  class Fixed_alpha_shape_vertex_base_with_info_3
    : public Vb
  {
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


  void regular_triangulation_reconstruction(
      median_skeleton& output,
      const structurer::parameters& params )
  {
    tbb::task_scheduler_init init;
    const auto natoms = output.get_number_of_atoms();

    std::vector< rt::Weighted_point > wpoints( natoms );
    std::vector< median_skeleton::atom_index > vinfos( natoms );
    # pragma omp parallel for schedule(static)
    for( median_skeleton::atom_index i = 0; i < natoms; ++ i )
      {
        const auto& atom = output.get_atom_by_index( i );
        wpoints[ i ] = rt::Weighted_point( rt::Bare_point( atom.x, atom.y, atom.z ), atom.w * atom.w );
        vinfos[ i ] = i;
      }

    graphics_origin::geometry::aabox bbox;
    output.compute_centers_bounding_box( bbox );
    bbox.m_hsides += 1e-6;
    rt::Lock_data_structure locking_datastructure(
        CGAL::Bbox_3(
            bbox.m_center.x - bbox.m_hsides.x, bbox.m_center.y - bbox.m_hsides.y, bbox.m_center.z - bbox.m_hsides.z,
            bbox.m_center.x + bbox.m_hsides.x, bbox.m_center.y + bbox.m_hsides.y, bbox.m_center.z + bbox.m_hsides.z ),
        8);

    rt regular_tetrahedrization(
        boost::make_zip_iterator(boost::make_tuple( wpoints.begin(), vinfos.begin() )),
        boost::make_zip_iterator(boost::make_tuple( wpoints.end()  , vinfos.end()   )),
        &locking_datastructure);

    fixed_alpha_shape alpha_shape( regular_tetrahedrization, 0 );

    if( params.m_build_faces )
      {
        auto nb_finite_facets = alpha_shape.number_of_finite_facets();
        output.reserve_faces( nb_finite_facets );
        output.reserve_links( nb_finite_facets * 3 );
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
                output.add( indices[0], indices[1], indices[2] );
              }
          }
      }
    else
      {
        output.reserve_links( alpha_shape.number_of_finite_edges() );
        for( auto eit = alpha_shape.edges_begin(), eitend = alpha_shape.edges_end();
            eit != eitend; ++ eit )
          {
            auto type = alpha_shape.classify( *eit );
            if( type == fixed_alpha_shape::REGULAR || type == fixed_alpha_shape::SINGULAR )
              {
                output.add( eit->first->vertex( eit->second )->info(), eit->first->vertex( eit->third )->info() );
              }
          }
      }
  }


END_MP_NAMESPACE
