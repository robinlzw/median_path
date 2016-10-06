# ifndef MEDIAN_PATH_ATOMIZATION_H_
# define MEDIAN_PATH_ATOMIZATION_H_

# include "median_skeleton.h"
# include <graphics-origin/geometry/mesh.h>

namespace median_path {

  typedef graphics_origin::geometry::mesh skeletonizable_shape;

  namespace atomizer {

    struct method_category {};
    struct empty_method :
        public method_category {};
    struct voronoi_method :
        public method_category {};
    struct shrinking_ball_method :
        public method_category {};

    /**@brief Sampling used for atomization.
     *
     * A sampling of the shape is used to perform an atomization in practice.
     * This yields to an approximation of the exact set of atoms. However,
     * since we can only represent a finite set of atoms in a computer, an
     * approximation is needed in the general case.
     *
     * Some atomization methods use only vertex sampling, while others use
     * face sampling or even both vertex and face sampling. Each atomization
     * method will define which type of sampling it uses, under the name
     * sampling_type. The samples used to compute an atom are stored inside
     * an atom property. The index of this property is stored in the atomization
     * method in the variable atom_to_sampling_property_index. The type
     * of elements of that property depends on the sampling method and of the
     * atomization method used and is named atom_to_sampling_type.
     */
    struct sampling_category {};
    /**@brief Tag for atomization relying on vertex sampling.
     *
     * When the type sampling_type inside an atomization is defined to
     * be vertex_sampling, it indicates that the atomization rely on
     * vertex sampling to compute atoms. The atom property storing the
     * samples used will then store vertex indices.
     * \sa sampling_category */
    struct vertex_sampling :
        public sampling_category {};
    /**@brief Tag for atomization relying on triangle sampling.
     *
     * When the type sampling_type inside an atomization is defined to
     * be triangle_sampling, it indicates that the atomization rely on
     * triangle sampling to compute atoms. The atom property storing the
     * samples used will then store triangle indices.
     * \sa sampling_category */
    struct triangle_sampling :
        public sampling_category {};
    /**@brief Tag for atomization that does not use sampling.
     *
     * When the type sampling_type inside an atomization is defined to
     * be no_sampling, it indicates that the atomization does not
     * perform sampling at all. Thus, this tag will be used by atomizations
     * that does not compute atoms, such as no_atomization.
     * \sa sampling_category */
    struct no_sampling :
        public sampling_category {};

    struct no_atomization {

      typedef empty_method method_type;
      typedef no_sampling sampling_type;
      struct parameters_type {
        typedef no_atomization atomizer_type;
      };
      struct atom_to_sampling_type {};

      no_atomization( const parameters_type& parameters, const skeletonizable_shape& shape, median_skeleton& result )
        : atom_to_sampling_property_index{0}
      {
        (void)parameters;
        (void)shape;
        (void)result;
      }

      median_skeleton::atom_property_index atom_to_sampling_property_index;
    };

    struct shrinking_ball_vertex_constant_initial_radius {
      typedef shrinking_ball_method method_type;
      typedef vertex_sampling sampling_type;
      struct parameters_type {
        typedef shrinking_ball_vertex_constant_initial_radius atomizer_type;
        const real constant_initial_radius_ratio = 0.6;
        const real radius_variation_threshold_ratio = 0.0001;
      };

      // The first index is for the contact vertex whose normal is normal to
      // the atom.
      typedef std::array< median_skeleton::atom_index, 2 > atom_to_sampling_type;

      shrinking_ball_vertex_constant_initial_radius(
          const parameters_type& parameters,
          const skeletonizable_shape& shape,
          median_skeleton& result );

      parameters_type parameters;
      median_skeleton::atom_property_index atom_to_sampling_property_index;
    };
  }

# ifdef MP_USE_CONCEPTS
  template< typename T >
  concept bool Atomizer() {

    return requires {
      typename T::method_type;
      typename T::sampling_type;
      typename T::parameters_type;
      typename T::atom_to_sampling_type;

      requires std::is_base_of<atomizer::method_category, typename T::method_type>::value;
      requires std::is_base_of<atomizer::sampling_category, typename T::sampling_type>::value;
    } && requires( T t ) {

      { t.atom_to_sampling_property_index } -> median_skeleton::atom_property_index;

    } && requires( typename T::parameters_type const& parameters, skeletonizable_shape const& shape, median_skeleton& result ) {
        T( parameters, shape, result);
    };
  }

  template< typename T >
  concept bool Atomizer_parameters() {
    return requires {
      typename T::atomizer_type;
      requires std::is_default_constructible< T >::value;
      requires std::is_same< T, typename T::atomizer_type::parameters_type >::value;
    };
  }
# endif
}

# endif
