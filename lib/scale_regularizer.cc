/* Created on: Apr 12, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# include "../median-path/regularization.h"
# include <graphics-origin/geometry/box.h>
# define CGAL_LINKED_WITH_TBB
# include <tbb/task_scheduler_init.h>
# include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
# include <CGAL/Triangulation_vertex_base_with_info_3.h>
# include <CGAL/Regular_triangulation_euclidean_traits_3.h>
# include <CGAL/Regular_triangulation_3.h>

BEGIN_MP_NAMESPACE

  typedef CGAL::Epick rt_kernel;
  typedef CGAL::Regular_triangulation_euclidean_traits_3< rt_kernel > rt_traits;
  typedef CGAL::Triangulation_vertex_base_with_info_3< median_skeleton::atom_index, rt_traits > rt_vertex_base;
  typedef CGAL::Triangulation_cell_base_3< rt_traits > rt_cell_base;
  typedef CGAL::Triangulation_data_structure_3< rt_vertex_base, rt_cell_base, CGAL::Parallel_tag > rt_datastructure;
  typedef CGAL::Regular_triangulation_3< rt_traits, rt_datastructure > rt;

  void scale_regularizer(
      median_skeleton& skeleton,
      real scale )
  {
    tbb::task_scheduler_init init;
    const auto natoms = skeleton.get_number_of_atoms();

    std::vector< rt::Weighted_point > wpoints( natoms );
    std::vector< median_skeleton::atom_index > vinfos( natoms );
    scale *= scale;
    # pragma omp parallel for schedule(static)
    for( median_skeleton::atom_index i = 0; i < natoms; ++ i )
      {
        const auto& atom = skeleton.get_atom_by_index( i );
        wpoints[ i ] = rt::Weighted_point( rt::Bare_point( atom.x, atom.y, atom.z ), scale * atom.w * atom.w );
        vinfos[ i ] = i;
      }

    graphics_origin::geometry::aabox bbox = skeleton.compute_centers_bounding_box();
    bbox.hsides += 1e-6;
    rt::Lock_data_structure locking_datastructure(
        CGAL::Bbox_3(
            bbox.center.x - bbox.hsides.x, bbox.center.y - bbox.hsides.y, bbox.center.z - bbox.hsides.z,
            bbox.center.x + bbox.hsides.x, bbox.center.y + bbox.hsides.y, bbox.center.z + bbox.hsides.z ),
        8);

    rt regular_tetrahedrization(
        boost::make_zip_iterator(boost::make_tuple( wpoints.begin(), vinfos.begin() )),
        boost::make_zip_iterator(boost::make_tuple( wpoints.end()  , vinfos.end()   )),
        &locking_datastructure);

    std::vector< bool > delete_flags( natoms, true );
    for( auto it = regular_tetrahedrization.finite_vertices_begin(),
        end = regular_tetrahedrization.finite_vertices_end(); it != end; ++ it )
      {
        delete_flags[ it->info() ] = false;
      }

    skeleton.remove_atoms( [&skeleton, &delete_flags]( median_skeleton::atom& e )
     {
        return delete_flags[ skeleton.get_index( e ) ];
     }, true );
  }

END_MP_NAMESPACE
