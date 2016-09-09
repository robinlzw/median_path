# ifndef NEW_SKELETONIZATION_H_
# define NEW_SKELETONIZATION_H_

# include "median_path.h"
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
     * atomization method used.
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

      no_atomization( const parameters_type& parameters, const skeletonizable_shape& shape, median_skeleton& result )
      {
        (void)parameters;
        (void)shape;
        (void)result;
      }
    };

    struct shrinking_ball_vertex_constant_initial_radius {
      typedef shrinking_ball_method method_type;
      typedef vertex_sampling sampling_type;
      struct parameters_type {
        typedef shrinking_ball_vertex_constant_initial_radius atomizer_type;
        const real constant_initial_radius_ratio;
        const real radius_variation_threshold_ratio;
      };
      struct atom_to_sampling_type {
        // first contact vertex, its normal is normal to the atom
        uint32_t first_vertex;
        // second contact vertex
        uint32_t second_vertex;
      };

      shrinking_ball_vertex_constant_initial_radius(
          const parameters_type& parameters,
          const skeletonizable_shape& shape,
          median_skeleton& result )
      : parameters{ parameters }, kdtree{ shape }
      {
        result.clear( shape.n_vertices() );
      }

      parameters_type parameters;
      graphics_origin::geometry::mesh_vertices_kdtree kdtree;

    };

    struct shrinking_ball_vertex {
      typedef shrinking_ball_method method_type;
      typedef vertex_sampling sampling_type;
      struct parameters_type {
        typedef shrinking_ball_vertex atomizer_type;
        enum { raytracing, constant } initial_radius_method;
        real constant_radius_ratio;
        real radius_variation_threshold; //fixme: should be a ratio?
      };




      median_skeleton::atom_property_index atom_to_sampling_property_index;
    };
  }

  namespace structurer2 {

    struct no_structuration {
      struct parameters_type {
        typedef no_structuration structurer_type;
      };
      template< typename atomizer_type >
      no_structuration(
          const parameters_type& parameters,
          const skeletonizable_shape& shape,
          atomizer_type& atomizer,
          median_skeleton& result )
      {
        (void)parameters;
        (void)shape;
        (void)atomizer;
        (void)result;
      }

    };

  }

  namespace regularizer2 {

    struct no_regularization {

      struct parameters_type {
        typedef no_regularization regularizer_type;
      };

      template<
              typename atomizer_type,
              typename structurer_type >
      no_regularization(
          const parameters_type& parameters,
          const skeletonizable_shape& shape,
          atomizer_type& atomizer,
          structurer_type& structurer,
          median_skeleton& result )
      {
        (void)parameters;
        (void)shape;
        (void)atomizer;
        (void)structurer;
        (void)result;
      }
    };
  }


  /**
   * A skeletonization is a set of three main operations:
   * - atomizer; the extraction of a finite set of atoms from a shape
   * - structurer; the connection of atom centers to reflect the structure of
   *   the shape with links and faces
   * - regularizer; the filtering of atoms, links and faces to remove noise.
   *
   * Some (most) structuring and regularizing operations heavily rely on
   * intermediate data computed during the atom extraction. Also, not all
   * combination of atomizer, structurer and regularizer is valid. Moreover,
   * implementations of the same steps can have very different set of
   * parameters and inputs.
   *
   * For such reasons, designing atomizer, structurer, regularizer and
   * skeletonizer is not a trivial task.
   *
   * We will suppose that the regularization always takes place after the
   * structuration. Indeed, if a regularization does not rely on the skeletal
   * structure, the skeletal structure will not be in a valid state (holes).
   * Thus, another structuration must be done after regularization. In that
   * case, structurations depending on intermediate data computed during the
   * atom extraction will not be applicable anymore, because of atom removal.
   * It could be worth to note that some regularization can be directly
   * included into the atomization, e.g.: the polar ball atomization (a Voronoi
   * atomization following by the filtering of any Voronoi vertex that is not
   * a pole).
   *
   * Now, let's start with the interfaces we would like for those objects:
   *
   * // skeletonizer type is templated on the operations to do
   * typedef skeletonizer< atomizer, structurer, regularizer > skeletonizer_type;
   * // a skeletonizer is constructed with operations, in order to set their parameters
   * skeletonizer_type myskeletonizer( atomizer{...}, structurer{...}, regularizer{...});
   * // the skeletonizer can return a new skeleton...
   * medial_skeleton s = myskeletonizer.skeletonize( skeletonizable_shape{} );
   * // ...or fill an existing one.
   * myskeletonizer.skeletonize( skeletonizable_shape{}, s );
   * // a helper function should allow the following.
   * s = skeletonize( atomizer{...}, structure{...}, regularizer{...}, skeletonizable_shape );
   *
   * The two main issues are:
   * - how to compute and to pass intermediate data from one operation to another?
   * - how to enforce valid combinations of atomization, structuration and regularization?
   *
   * We will have a limited set of operation implementations. This set can be
   * improved in the future, adding more constraints, more data to compute.
   * Thus, instead of trying to design classes that can handle any possible
   * case, I prefer to handle the available cases. Also, I will rather choose
   * simplicity over performance, e.g.:
   * - in some cases it might be useless to compute some data since next steps
   * will not use it. Checking when it is necessary and providing one code
   * path with and one without the computation is more complex than computing
   * it every time.
   * - in some cases it might be more fast to compute some data during the
   * atomization than latter in the regularization. Again, checking when it
   * is necessary and providing more than one execution paths is more complex
   * than letting further step computing those data.
   *
   * Chaque operation doit connaitre le type des autres operations afin d'avoir
   * acces aux typedefs.
   */

  /**
   * This class is not meant to be thread-safe. Be sure to not make any
   * concurrent call to its methods.
   */
  template<
    typename atomizer_type,
    typename structurer_type,
    typename regularizer_type >
  class skeletonizer2 {
  public:

    // std::is_base_of...
    skeletonizer2(
        const typename atomizer_type::parameters_type& atomizer_parameters,
        const typename structurer_type::parameters_type& structurer_parameters,
        const typename regularizer_type::parameters_type& regularizer_parameters ) noexcept
      : atomizer_parameters{ atomizer_parameters }, structurer_parameters{ structurer_parameters }, regularizer_parameters{ regularizer_parameters }
    {}

    median_skeleton skeletonize( const skeletonizable_shape& shape )
    {
      median_skeleton result;
      skeletonize( shape, result );
      return result;
    }

    void skeletonize( const skeletonizable_shape& shape, median_skeleton& result )
    {
      //fixme: I am a little concerned about the life extent of (potentially
      // heavy) data stored inside atomizer, structurer and regularizer. It
      // should be release as soon as it become useless. Only the structurer and
      // the regularizer could tell if the atomizer data is now useless. Only the
      // regularizer could tell if the structurer data is now useless. Thus,
      // releasing data can only be done inside the structurer and the regularizer.
      atomizer_type atomizer( atomizer_parameters, shape, result );
      structurer_type structurer( structurer_parameters, shape, atomizer, result );
      regularizer_type regularizer( regularizer_parameters, shape, atomizer, structurer, result );
    }

    void update_atomizer( const typename atomizer_type::parameters_type& params )
    {
      atomizer_parameters = params;
    }

    void update_structurer( const typename structurer_type::parameters_type& params )
    {
      structurer_parameters = params;
    }

    void update_regularizer( const typename regularizer_type::parameters_type& params )
    {
      regularizer_parameters = params;
    }

  private:
    typename atomizer_type::parameters_type atomizer_parameters;
    typename structurer_type::parameters_type structurer_parameters;
    typename regularizer_type::parameters_type regularizer_parameters;
  };

  template<
    typename atomizer_parameters_type,
    typename structurer_parameters_type,
    typename regularizer_parameters_type>
  median_skeleton skeletonize(
      const atomizer_parameters_type& atomizer,
      const structurer_parameters_type& structurer,
      const regularizer_parameters_type& regularizer,
      const skeletonizable_shape& shape )
  {
    typedef skeletonizer2<
        typename atomizer_parameters_type::atomizer_type,
        typename structurer_parameters_type::structurer_type,
        typename regularizer_parameters_type::regularizer_type > skeletonizer_type;
    return skeletonizer_type( atomizer, structurer, regularizer ).skeletonize( shape );
  };
}
# endif /* NEW_SKELETONIZATION_H_ */
