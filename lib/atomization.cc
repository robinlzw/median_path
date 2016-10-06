# include "../median-path/atomization.h"

namespace median_path {

  static inline real
  compute_radius_of_tangent_ball(
      const real* tangent_position,
      const real* tangent_normal,
      const real* contact_position )
  {
    real diff[3] = {
        tangent_position[0] - contact_position[0],
        tangent_position[1] - contact_position[1],
        tangent_position[2] - contact_position[2] };
    return ( diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2] )
        / ( real(2) * std::abs( diff[0] * tangent_normal[0] + diff[1] * tangent_normal[1] + diff[2] * tangent_normal[2] ));
  }

  namespace atomizer {

    shrinking_ball_vertex_constant_initial_radius::shrinking_ball_vertex_constant_initial_radius(
        const parameters_type& parameters,
        const skeletonizable_shape& shape,
        median_skeleton& result )
    : parameters{ parameters }
    {
      graphics_origin::geometry::mesh_vertices_kdtree kdtree( shape );

      const auto nvertices = shape.n_vertices();
      const real min_bbox_length = real(2.0) * min(kdtree.get_bounding_box().hsides);
      const real initial_radius = min_bbox_length * parameters.constant_initial_radius_ratio;
      const real radius_variation_threshold = min_bbox_length * parameters.radius_variation_threshold_ratio;

      result.clear( nvertices, 0, 0 );
      base_property_buffer& mapping = result.add_atom_property<atom_to_sampling_type>( "atom_to_sampling" );
      atom_to_sampling_property_index = result.get_atom_property_index( mapping );

      # pragma omp parallel
      {
        typedef graphics_origin::geometry::mesh_vertices_kdtree::vertex_index vertex_index;

        vertex_index indices[2];
        real squared_distances[2];

        # pragma omp for schedule(dynamic)
        for( vertex_index i = 0; i < nvertices; ++ i )
          {
            const real* vertex_position = kdtree.get_point( i );
            const real* vertex_normal = &shape.normal( graphics_origin::geometry::mesh::VertexHandle(i) )[0];

            real next_radius = initial_radius;
            vertex_index other_index;
            vec3 center;
            real radius;

            do
              {
                radius = next_radius;
                center[0] = vertex_position[0] - radius * vertex_normal[0];
                center[1] = vertex_position[1] - radius * vertex_normal[1];
                center[2] = vertex_position[2] - radius * vertex_normal[2];

                kdtree.k_nearest_vertices( center, 2, indices, squared_distances );
                other_index = indices[0] == i ? indices[1] : indices[0];
                next_radius = compute_radius_of_tangent_ball(
                    vertex_position,
                    vertex_normal,
                    kdtree.get_point( other_index ));
              }
            while( std::abs( next_radius - radius ) > radius_variation_threshold );

            if( std::isfinite( center.x ) && std::isfinite( center.y )
              && std::isfinite( center.z) && std::isfinite( radius ) )
              {
                median_skeleton::atom_handle handle;
                # pragma omp critical
                handle = result.add( median_skeleton::atom( center, radius ) );

                mapping.get<atom_to_sampling_type>( result.get_index( handle ) ) = { i, other_index };
              }
          }
      }
    }

  }

}
