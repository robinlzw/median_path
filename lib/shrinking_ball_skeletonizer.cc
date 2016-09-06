/*  Created on: Mar 15, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "../median-path/skeletonization.h"
# include <graphics-origin/geometry/ray.h>

BEGIN_MP_NAMESPACE

  skeletonizer::shrinking_balls_parameters::shrinking_balls_parameters()
    : m_radius_method{ RAYTRACING },
      m_constant_radius_ratio{0.6},
      m_min_radius_variation{1e-6}
  {}

  static inline
  real get_squared_distance( const real* a, const real* b )
  {
    real result = a[0] - b[0];
    real accu = a[1] - b[1];
    result *= result;
    accu *= accu;
    result += accu;
    accu = a[2] - b[2];
    accu *= accu;
    result += accu;
    return result;
  }

  void delaunay_reconstruction(
      median_skeleton& skeleton,
      graphics_origin::geometry::mesh_spatial_optimization& msp,
      std::vector< std::vector< uint32_t > >& vertex_to_atoms,
      const structurer::parameters& params );

  static inline real
  compute_radius( const real* sample, const real* point, const real* normal )
  {
    real diff[3] = { sample[0] - point[0], sample[1] - point[1], sample[2] - point[2] };
    return (diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2] )
        / (  real(2) * std::abs( diff[0] * normal[0] + diff[1] * normal[1] + diff[2] * normal[2] ));
  }

  void shrinking_ball_skeletonizer(
     graphics_origin::geometry::mesh_spatial_optimization& input,
     median_skeleton& output,
     const skeletonizer::parameters& params )
  {
    input.build_kdtree();
    if( params.m_shrinking_ball.m_radius_method == skeletonizer::shrinking_balls_parameters::RAYTRACING )
      input.build_bvh();
    const auto nsamples = input.kdtree_get_point_count();
    output.clear( nsamples, 0, 0 );

    const auto& bbox = input.get_bounding_box();
    const real global_initial_radius =
        real(2.0) * params.m_shrinking_ball.m_constant_radius_ratio *
        std::min( bbox.hsides.x, std::min( bbox.hsides.y, bbox.hsides.z ) );

    std::vector< std::vector< uint32_t > > vertex_to_atoms;
    bool keep_vertex_to_atoms = params.m_build_topology
        && (params.m_structurer_parameters.m_topology_method == structurer::parameters::DELAUNAY_RECONSTRUCTION);

    if( keep_vertex_to_atoms )
      vertex_to_atoms.resize( nsamples );

    # pragma omp parallel
    {
      size_t indices[2];
      real sdistances[2];

      # pragma omp for schedule(dynamic)
      for( uint32_t i = 0; i < nsamples; ++ i )
        {
          const real* sample_position = input.get_point( i );
          const real* sample_normal = input.get_normal( i );

          real radius = global_initial_radius;
          if( params.m_shrinking_ball.m_radius_method == skeletonizer::shrinking_balls_parameters::RAYTRACING )
            {
              real distance;
              size_t face_idx;
              if( input.intersect(
                  graphics_origin::geometry::ray(
                      vec3{ sample_position[0], sample_position[1], sample_position[2]},
                      vec3{  -sample_normal[0],  -sample_normal[1],  -sample_normal[2]}),
                  distance, face_idx ) )
                {
                  // ok, we have an intersection, but the distance may be too short
                  auto fvit = input.get_geometry().fv_begin(
                      graphics_origin::geometry::mesh::FaceHandle( face_idx ) );
                  distance = get_squared_distance( sample_position, input.get_point( fvit->idx() ) );
                  ++fvit;
                  distance = std::min( distance, get_squared_distance( sample_position, input.get_point( fvit->idx() ) ) );
                  ++fvit;
                  distance = std::min( distance, get_squared_distance( sample_position, input.get_point( fvit->idx() ) ) );
                  radius = std::sqrt( distance );
                }
            }

          vec3 center{
            sample_position[0] - radius * sample_normal[0],
            sample_position[1] - radius * sample_normal[1],
            sample_position[2] - radius * sample_normal[2] };

          input.k_nearest_vertices( center, 2, indices, sdistances );
          const real* other_sample = input.get_point( indices[0] == i ? indices[1] : indices[0] );
          real next_radius = compute_radius( sample_position, other_sample, sample_normal );
          while( std::abs( next_radius - radius ) > params.m_shrinking_ball.m_min_radius_variation )
            {
              radius = next_radius;
              center = vec3 {
                sample_position[0] - radius * sample_normal[0],
                sample_position[1] - radius * sample_normal[1],
                sample_position[2] - radius * sample_normal[2] };

              input.k_nearest_vertices( center, 2, indices, sdistances );
              other_sample = input.get_point( indices[0] == i ? indices[1] : indices[0] );
              next_radius = compute_radius( sample_position, other_sample, sample_normal );
            }

          if( std::isfinite( center.x ) && std::isfinite( center.y ) && std::isfinite( center.z ) && std::isfinite( radius ) )
            {
              # pragma omp critical
              {
                if( keep_vertex_to_atoms )
                  {
                    auto id = output.get_number_of_atoms();
                    vertex_to_atoms[ indices[0] ].push_back( id );
                    vertex_to_atoms[ indices[1] ].push_back( id );
                  }
                output.add( median_skeleton::atom( center, radius ) );
              }
            }
        }
    }

    if( keep_vertex_to_atoms )
      {
        delaunay_reconstruction( output, input, vertex_to_atoms, params.m_structurer_parameters );
      }
  }


END_MP_NAMESPACE
