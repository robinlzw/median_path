# include "../median-path/new_skeletonization.h"

# include <graphics-origin/tools/log.h>

# include <boost/program_options.hpp>

# include <sys/ioctl.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <iostream>

static const char* version_string = "Skeletonizer command line tool v1.0 @2016 Thomas Delame";

static const char* help_string =
    "Skeletonizer\n"\
    "============\n"\
    "TODO";

static uint32_t get_console_line_length() {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return w.ws_col;
}

namespace po = boost::program_options;

struct application_parameters {
  std::vector< std::string > input_filenames;
  std::string output_directory;
  std::string output_name;
  std::string output_extension;

  skeletonizer_parameters skeletonization;

  application_parameters( int argc, char* argv[] )
  {
    auto line_length = get_console_line_length();
    po::options_description generic(
        "Generic options", line_length, line_length > 10 ? line_length -10 : line_length );
    generic.add_options()
        ("version,v", "print version string")
        ("help,h", "produce help message")
        ("output_directory,d", po::value<std::string>(&output_directory), "skeleton output directory")
        ("output_name,o", po::value<std::string>(&output_name),"output name of the skeleton file when there is only one input mesh")
        ("format,f", po::value<std::string>(&output_extension),"output file format (median, balls, moff and web)");

    po::options_description visible;
    visible.add(generic);

    po::options_description hidden( "Hidden Options");
    hidden.add_options()("input,i", po::value<std::vector<std::string>>(&input_filenames), "input mesh file names");

    po::options_description cmdline_options;
    cmdline_options.add(visible).add(hidden);

    po::positional_options_description p;
    p.add("input", -1);

    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(),
        vm);
    po::notify(vm);


    if( vm.count("version" ) )
      std::cout<< version_string << std::endl;

    if( vm.count("help") )
      {
        std::cout<< help_string << visible << std::endl;
        exit( EXIT_SUCCESS );
      }

    for( auto iterator = input_filenames.begin(), end = input_filenames.end();
        iterator != end; )
      {
        if( graphics_origin::geometry::has_a_mesh_file_extension( *iterator ) )
          ++iterator;
        else
          {
            LOG( error, "file " << *iterator << " is not a known mesh file");
            iterator = input_filenames.erase( iterator );
          }
      }

    if( input_filenames.empty() )
      {
        LOG( error, "no input mesh file names");
        std::cout << help_string << visible << std::endl;
        exit( EXIT_FAILURE );
      }
    if( input_filenames.size() > 1 )
      output_name.clear();
  }

  std::string
  get_output_filename( const std::string& input_filename )
  {
    if( !output_name.empty() )
      return output_name;
    if( output_directory.empty() )
      return graphics_origin::tools::get_directory_name( input_filename ) + "/"
          + graphics_origin::tools::get_stem( input_filename ) + "." + output_extension;
    return output_directory + "/" + graphics_origin::tools::get_stem( input_filename ) + "." + output_extension;
  }
};

median_path::median_skeleton
no_atomization_skeletonizer(
    const skeletonizer_parameters& parameters,
    const median_path::skeletonizable_shape& shape ) {

}

median_path::median_skeleton
polar_ball_skeletonizer(
    const skeletonizer_parameters& parameters,
    const median_path::skeletonizable_shape& shape ) {

}

median_path::median_skeleton
shrinking_ball_skeletonizer(
    const skeletonizer_parameters& parameters,
    const median_path::skeletonizable_shape& shape ) {

}

median_path::median_skeleton
voronoi_ball_skeletonizer(
    const skeletonizer_parameters& parameters,
    const median_path::skeletonizable_shape& shape ) {

}

median_path::median_skeleton
skeletonizer_helper(
    const skeletonizer_parameters& parameters,
    const median_path::skeletonizable_shape& shape ) {
  switch( parameters.atomization ) {
    case skeletonizer_parameters::atomization_type::no_atomization:
      return no_atomization_skeletonizer( parameters, shape );
    case skeletonizer_parameters::atomization_type::polar_ball:
      return polar_ball_skeletonizer( parameters, shape );
    case skeletonizer_parameters::atomization_type::shrinking_ball:
      return shrinking_ball_skeletonizer( parameters, shape );
    case skeletonizer_parameters::atomization_type::voronoi_ball:
      return voronoi_ball_skeletonizer( parameters, shape );
  }
  throw std::logic_error("unexpected atomization type");
}

int main( int argc, char* argv[] )
{
  int return_value = EXIT_SUCCESS;
  try
    {
      application_parameters parameters( argc, argv );
      for( auto& filename : parameters.input_filenames )
        {
          median_path::skeletonizable_shape shape(filename);
          median_path::median_skeleton result = skeletonizer_helper( parameters.skeletonization, shape );
          result.save( parameters.get_output_filename( filename ) );
        }
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
