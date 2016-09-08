# ifndef NEW_STRUCTURATION_H_
# define NEW_STRUCTURATION_H_

namespace median_path {

  typedef enum {
     weighted_zero_shape,
     delaunay,
     powershape,
     voronoi,
     no_structuration } structuration_method;

   struct voronoi_structuration_parameters {
   };

   struct powershape_structuration_parameters {
   };

   struct delaunay_structuration_parameters {
   };

   struct weighted_zero_shape_structuration_parameters {
   };

   template< typename structuration_parameters >
   void structurize(
       median_skeleton& skeleton,
       const structuration_parameters& params )
   {
     static_assert(
         implementation_required<structuration_parameters>::value,
         "Please, provide a structuration implementation for this type of parameters");
   }

   template<>
   void structurize( median_skeleton& skeleton, const no_structuration& params )
   {
     (void)skeleton;
     (void)params;
   }

   /**
    *
    * optimal_skeletonizer = skeletonizer<weighted_zero_shape_parameters>( parameters );
    *
    * skeleton = optimal_skeletonizer(shape);
    * skeleton = skeletonize(shape, parameters);
    *
    *
    *
    */
}

#endif /* NEW_STRUCTURATION_H_ */
