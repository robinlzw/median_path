# ifndef NEW_SKELETONIZATION_H_
# define NEW_SKELETONIZATION_H_

# include "atomization.h"
# include "structuration.h"

namespace median_path {

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
# ifdef MP_USE_CONCEPTS
    requires Atomizer<atomizer_type>()
          && Structurer<structurer_type, atomizer_type>()
# endif
  class skeletonizer2 {
  public:

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
# ifdef MP_USE_CONCEPTS
    requires Atomizer_parameters< atomizer_parameters_type >()
          && Structurer_parameters< structurer_parameters_type >()
# endif
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
