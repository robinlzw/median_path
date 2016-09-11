# include "../median-path/new_skeletonization.h"

# include <graphics-origin/tools/log.h>

# include <iostream>
# include <stdlib.h>

int main( int argc, char* argv[] )
{
  (void)argc;
  (void)argv;
  int return_value = EXIT_SUCCESS;
  try
    {
      median_path::skeletonizable_shape shape;

      typedef median_path::skeletonizer2<
          median_path::atomizer::no_atomization,
          median_path::structurer2::no_structuration,
          median_path::regularizer2::no_regularization >
      idle_skeletonizer;

      idle_skeletonizer skeletonizer(
          median_path::atomizer::no_atomization::parameters_type {},
          median_path::structurer2::no_structuration::parameters_type {},
          median_path::regularizer2::no_regularization::parameters_type {} );

      median_path::median_skeleton result = skeletonizer.skeletonize( shape );

      // simpler syntax with the function form
      median_path::median_skeleton another_result =
          median_path::skeletonize(
              median_path::atomizer::shrinking_ball_vertex_constant_initial_radius::parameters_type {},
              median_path::structurer2::no_structuration::parameters_type {},
              median_path::regularizer2::no_regularization::parameters_type {},
              shape );

    }
  catch( std::exception& e )
    {
      LOG( fatal, "exception caught: " << e.what() );
      std::cerr << "exception caught: " << e.what() << std::endl;
      return_value = EXIT_FAILURE;
    }
  graphics_origin::tools::flush_log();
  return return_value;
}
