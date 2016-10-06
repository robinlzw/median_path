# ifndef MEDIAN_PATH_STRUCTURATION_H_
# define MEDIAN_PATH_STRUCTURATION_H_

# include "atomization.h"

namespace median_path {

  typedef graphics_origin::geometry::mesh skeletonizable_shape;


/**@brief Structure a median skeleton.
 *
 * This class structures a median skeleton, that is it reconstruct the topology
 * of the shape described by the atom in the form of links and triangles
 * connecting skeleton points.
 *
 * Currently, 4 methods are available, but not all of them can be applied apart
 * from a skeletonization algorithm:
 * - weighted alpha shape, can be applied on any median skeleton, at any time
 * - delaunay reconstruction, can be applied on any median skeleton, at any time
 * - powershape, should be applied during Voronoi or polar skeletonization only
 * - voronoi, should be applied during Voronoi skeletonization only. */
class structurer {
public:

  /**@brief Parameters to tune a structuration.
   *
   * Parameters of a structuration process. */
  struct parameters {
    parameters();
    typedef enum {
      WEIGHTED_ALPHA_SHAPE,
      DELAUNAY_RECONSTRUCTION,
      POWERSHAPE,
      VORONOI
    } topology_method;
    topology_method m_topology_method;
    bool m_build_faces;
    bool m_neighbors_should_intersect;
  };

  structurer(
      median_skeleton& skeleton,
      const parameters& params = parameters() );

  const real& get_execution_time() const noexcept;
private:
  real m_execution_time;
};

  namespace structurer2 {

    struct no_structuration {
      struct parameters_type {
        typedef no_structuration structurer_type;
      };
      template< typename atomizer_type >
# ifdef MP_USE_CONCEPTS
        requires Atomizer<atomizer_type>()
# endif
      no_structuration(
          const parameters_type& parameters,
          const skeletonizable_shape& shape,
          atomizer_type& atomizer,
          median_skeleton& result )
      {
        (void)parameters; (void)shape; (void)atomizer; (void)result;
      }
    };


    struct weighted_zero_shape {
      struct parameters_type {
        typedef weighted_zero_shape structurer_type;
        const unsigned int grid_subdivisions = 8;
        const bool build_faces = true;
      };

      template< typename atomizer_type >
# ifdef MP_USE_CONCEPTS
        requires Atomizer<atomizer_type>()
# endif
      weighted_zero_shape( parameters_type const& parameters,
                           skeletonizable_shape const & shape,
                           atomizer_type& atomizer, median_skeleton& result )
       : weighted_zero_shape( parameters, result )
     {
        (void)atomizer; (void)shape;
     }

      weighted_zero_shape( parameters_type const& parameters, median_skeleton& result );
    };

    struct delaunay_reconstruction {
      struct parameters_type {
        typedef delaunay_reconstruction structurer_type;
        const bool build_faces = true;
        const bool neighbors_must_intersect = true;
      };

      template< typename atomizer_type >
# ifdef MP_USE_CONCEPTS
        requires Atomizer<atomizer_type>()
# endif
      delaunay_reconstruction( parameters_type const& parameters,
                               skeletonizable_shape const& shape,
                               atomizer_type& atomizer, median_skeleton& result );
    };
  }

# ifdef MP_USE_CONCEPTS
  template< typename T, typename U >
  concept bool Structurer() {
    return
        requires() {
          typename T::parameters_type;
        }
     && requires( typename T::parameters_type const& parameters,
                  skeletonizable_shape const& shape, U& atomizer,
                  median_skeleton& result ) {
          T( parameters, shape, atomizer, result );
        };
  }

  template< typename T >
  concept bool Structurer_parameters() {
    return requires() {
      typename T::structurer_type;
      requires std::is_default_constructible< T >::value;
      requires std::is_same< T, typename T::structurer_type::parameters_type >::value;
    };
  }
# endif

}
# include "detail/delaunay_reconstruction_implementation.h"
# endif 
