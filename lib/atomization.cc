# include "../median-path/new_skeletonization.h"

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
    : parameters{ parameters }, kdtree{ shape }
    {
      result.clear( shape.n_vertices(), 0, 0 );

    }

  }

}
