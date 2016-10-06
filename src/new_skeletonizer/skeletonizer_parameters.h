# ifndef MEDIAN_PATH_SKELETONIZER_PARAMETERS_H_
# define MEDIAN_PATH_SKELETONIZER_PARAMETERS_H_
# include "../../median-path/new_skeletonization.h"
namespace median_path {

  struct skeletonizer_parameters {
    typedef enum { no_atomization, shrinking_ball, voronoi_ball, polar_ball } atomization_type;
    typedef enum { no_structuration, weighted_zero_shape, delaunay_reconstruction, voronoi_diagram, powershape } structuration_type;
    typedef enum { no_regularization } regularization_type;

    struct atomization_parameters {
      real constant_initial_radius_ratio = 0.6;
      real radius_variation_threshold_ratio = 0.0001;
      unsigned int grid_subdivisions = 8;

      atomizer::no_atomization::parameters_type no_atomization_parameters() const {
        return atomizer::no_atomization::parameters_type{};
      }
      atomizer::shrinking_ball_vertex_constant_initial_radius::parameters_type shrinking_ball_parameters() const {
        return atomizer::shrinking_ball_vertex_constant_initial_radius::parameters_type{ constant_initial_radius_ratio, radius_variation_threshold_ratio };
      }
    };

    struct structuration_parameters {
      bool build_faces = true;
      bool neighbor_must_intersect = true;

      structurer2::no_structuration::parameters_type no_structuration_parameters() const;
      structurer2::weighted_zero_shape::parameters_type weighted_zero_shape_parameters() const;
      structurer2::delaunay_reconstruction::parameters_type delaunay_parameters() const;
    };

    atomization_type atomization;
    structuration_type structuration;

    atomization_parameters atomization_params;
    structuration_parameters structuration_params;
  };
}
# endif
