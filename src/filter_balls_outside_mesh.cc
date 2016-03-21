/*  Created on: Mar 21, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */

# include "../median-path/skeletonization.h"
# include <graphics-origin/geometry/mesh.h>
# include <graphics-origin/tools/filesystem.h>
# include <graphics-origin/tools/log.h>

# include <boost/program_options.hpp>
# include <sys/ioctl.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <iostream>

static const std::string version_string =
    "filter_balls_outside_mesh line tool v0.1 ©2016 Thomas Delame";

static const std::string help_string =
    "Purpose\n"\
    "=======\n"\
    "This tool removes all balls whose center are included inside a mesh.";

static uint32_t get_console_line_length()
{
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return w.ws_col;
}

namespace po = boost::program_options;

struct application_parameters {
  std::string input_mesh_filename;
  std::string input_skeleton_filename;
  std::string output_directory;
  std::string output_name;
  std::string output_extension;

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
        ("format,f", po::value<std::string>(&output_extension),"output file format (median, balls, moff and json)")
        ("mesh,m", po::value<std::string>(&input_mesh_filename),"file name of the input mesh")
        ("skeleton,s", po::value<std::string>(&input_skeleton_filename),"file name of the input skeleton")
        ;

    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv).options(generic).run(),
        vm);
    po::notify(vm);

    if( vm.count("help") )
      {
        std::cout<< help_string << generic << std::endl;
        exit( EXIT_SUCCESS );
      }

    if( vm.count("version" ) )
      std::cout<< version_string << std::endl;

    if( input_mesh_filename.empty() && input_skeleton_filename.empty() )
      {
        LOG( error, "missing input mesh or skeleton");
        std::cout<< help_string << generic << std::endl;
        exit( EXIT_FAILURE );
      }
  }

  std::string
  get_output_filename()
  {
    if( !output_name.empty() )
      return output_name;
    if( output_directory.empty() )
      return input_skeleton_filename;
    return output_directory + "/" + graphics_origin::tools::get_stem( input_skeleton_filename ) + "." + output_extension;
  }

};

int main( int argc, char* argv[] )
{
  int return_value = EXIT_SUCCESS;
  try
    {
      application_parameters params( argc, argv );

      graphics_origin::geometry::mesh mesh( params.input_mesh_filename );
      graphics_origin::geometry::mesh_spatial_optimization mso( mesh, true, true );
      median_path::median_skeleton skeleton( params.input_skeleton_filename );

      skeleton.remove( [&mso]( median_path::median_skeleton::atom& atom )
         {
            return !mso.contain( median_path::vec3(atom) );
         }, true );

      skeleton.save( params.get_output_filename() );
    }
  catch( std::exception& e )
    {
      LOG( fatal, "exception caught: " << e.what() );
      std::cerr << "exception caught: " << e.what() << std::endl;
      return_value = EXIT_FAILURE;
    }
  boost::log::core::get()->flush();
  return EXIT_SUCCESS;
}
