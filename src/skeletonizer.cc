/*  Created on: Mar 18, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "../median-path/skeletonization.h"
# include <graphics-origin/tools/filesystem.h>
# include <graphics-origin/tools/log.h>

# include <boost/program_options.hpp>
# include <sys/ioctl.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <iostream>

static const std::string version_string =
    "Skeletonizer command line tool v0.1 ©2016 Thomas Delame";

static const std::string help_string =
    "Skeletonizer\n"\
    "============\n"\
    "Skeletonize meshes into medial skeletons";

static uint32_t get_console_line_length()
{
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return w.ws_col;
}

namespace po = boost::program_options;

namespace std {
std::istream& operator>>( std::istream& in, median_path::skeletonizer::parameters::geometry_method& method )
{
  std::string token;
  in >> token;
  if( token == "shrinking_balls" )
    method = median_path::skeletonizer::parameters::SHRINKING_BALLS;
  else if( token == "polar_balls" )
    method = median_path::skeletonizer::parameters::POLAR_BALLS;
  else if( token == "voronoi_balls")
    method = median_path::skeletonizer::parameters::VORONOI_BALLS;
  else throw po::validation_error(
      po::validation_error::invalid_option_value,
      "geometry_method",
      token);
  return in;
}

std::istream& operator>>( std::istream& in, median_path::skeletonizer::parameters::topology_method& method )
{
  std::string token;
  in >> token;
  if( token == "regular_triangulation" )
    method = median_path::skeletonizer::parameters::REGULAR_TRIANGULATION;
  else if( token == "delaunay_reconstruction" )
    method = median_path::skeletonizer::parameters::DELAUNAY_RECONSTRUCTION;
  else if( token == "powershape")
    method = median_path::skeletonizer::parameters::POWERSHAPE;
  else throw po::validation_error(
      po::validation_error::invalid_option_value,
      "topology_method",
      token);
  return in;
}

std::istream& operator>>( std::istream& in, median_path::skeletonizer::shrinking_ball_parameters::initial_radius_method& method )
{
  std::string token;
  in >> token;
  if( token == "raytracing" )
    method = median_path::skeletonizer::shrinking_ball_parameters::RAYTRACING;
  else if( token == "constant" )
    method = median_path::skeletonizer::shrinking_ball_parameters::CONSTANT;
  else throw po::validation_error(
      po::validation_error::invalid_option_value,
      "initial_radius",
      token);
  return in;
}
}

struct application_parameters {
  std::vector< std::string > input_filename;
  std::string output_directory;
  std::string output_name;
  std::string output_extension;
  median_path::skeletonizer::parameters skeletonizer_parameters;


  application_parameters( int argc, char* argv[] )
    : skeletonizer_parameters{}
  {
    auto line_length = get_console_line_length();
    po::options_description generic(
        "Generic options", line_length, line_length > 10 ? line_length -10 : line_length );
    generic.add_options()
        ("version,v", "print version string")
        ("help,h", "produce help message")
        ("output_directory,d", po::value<std::string>(&output_directory), "skeleton output directory")
        ("output_name,o", po::value<std::string>(&output_name),"output name of the skeleton file when there is only one input mesh")
        ("format,f", po::value<std::string>(&output_extension),"output file format (median, balls, moff and json)");

    po::options_description skeletonization(
        "Skeletonization options", line_length, line_length > 10 ? line_length -10 : line_length );
    skeletonization.add_options()
        ("geometry_method,g",
            po::value<median_path::skeletonizer::parameters::geometry_method>(&skeletonizer_parameters.m_geometry_method)
            ->default_value(median_path::skeletonizer::parameters::SHRINKING_BALLS),
            "method to build the geometry. Possible values are:\n"
            "  * shrinking_balls, to use the shrinking ball method\n"
            "  * polar_balls, to use polar balls as in the PowerShape algorithm\n"
            "  * voronoi_balls, to use Voronoi balls")
        ("topology_method,t",
            po::value<median_path::skeletonizer::parameters::topology_method>(&skeletonizer_parameters.m_topology_method)
            ->default_value(median_path::skeletonizer::parameters::REGULAR_TRIANGULATION),
            "method to build the skeleton topology. Possible values are:\n"
            "  * regular_triangulation, to use a regular triangulation of atoms (fast)\n"
            "  * delaunay_reconstruction, to use Delaunay reconstruction method\n"
            "  * powershape, to use the method of the PowerShape (long)")
        ("volume_factor,f", po::value<median_path::real>(&skeletonizer_parameters.m_cluster_volume_factor)->default_value(0.005, "0.005"),
            "minimum volume ratio an intesecting ball should add to another one to not form a cluster")
        ("cluster_neighbors,n", po::value<median_path::median_skeleton::atom_index>(&skeletonizer_parameters.m_neighbors_for_cluster_detection)
            ->default_value(20),
            "number of closest neighbors to consider to detect atom clusters")
        ("build_topology", po::value<bool>(&skeletonizer_parameters.m_build_topology)->default_value(true),
            "switch on/off the skeleton topology construction")
        ("build_faces", po::value<bool>(&skeletonizer_parameters.m_build_faces)->default_value(true),
            "switch on/off the construction of triangular faces between atom centers")
        ("merge_clusters", po::value<bool>(&skeletonizer_parameters.m_merge_clusters)->default_value(true),
            "switch on/off the merge of atom clusters");

      po::options_description shrinking_balls(
        "Skeletonization options specific to the shrinking balls method", line_length, line_length > 10 ? line_length -10 : line_length );
      shrinking_balls.add_options()
        ("initial_radius", po::value<median_path::skeletonizer::shrinking_ball_parameters::initial_radius_method>(&skeletonizer_parameters.m_shrinking_ball.m_radius_method)
          ->default_value(median_path::skeletonizer::shrinking_ball_parameters::RAYTRACING),
          "method to compute the initial radius of a shrinking ball. Possible values are:\n"
          "  * raytracing, to cast a ray at a vertex in the opposite direction of its normal. The closest intersection point will be use to determine a correct initial radius.\n"
          "  * constant, use the same initial value for all balls")
        ("constant_radius", po::value<median_path::real>(&skeletonizer_parameters.m_shrinking_ball.m_constant_radius_ratio)->default_value(0.6, "0.6"),
          "ratio of the smallest dimension of the input shape bounding box to use as a global initial radius");
        ("radius_variation_threshold", po::value<median_path::real>(&skeletonizer_parameters.m_shrinking_ball.m_min_radius_variation)->default_value(1e-6, "1e-6"),
            "when the radius shrinking ball do not change more than this threshold, the algorithm stops for this ball.");


      po::options_description visible;
      visible.add(generic).add(skeletonization).add(shrinking_balls);

      po::options_description hidden( "Hidden Options");
      hidden.add_options()("input,i", po::value<std::vector<std::string>>(&input_filename), "input mesh file names");

      po::options_description cmdline_options;
      cmdline_options.add(visible).add(hidden);

      po::positional_options_description p;
      p.add("input", -1);

      po::variables_map vm;
      po::store(
          po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(),
          vm);
      po::notify(vm);

      if( vm.count("help") )
        {
          std::cout<< help_string << visible << std::endl;
          exit( EXIT_SUCCESS );
        }

      if( vm.count("version" ) )
        std::cout<< version_string << std::endl;

      for( auto iterator = input_filename.begin(), end = input_filename.end();
          iterator != end; )
        {
          if( graphics_origin::geometry::has_a_mesh_file_extension( *iterator ) )
            ++iterator;
          else
            {
              LOG( error, "file " << *iterator << " is not a known mesh file");
              iterator = input_filename.erase( iterator );
            }
        }

      if( input_filename.empty() )
        {
          LOG( error, "no input mesh file names");
          std::cout << help_string << visible << std::endl;
          exit( EXIT_FAILURE );
        }
      if( input_filename.size() > 1 )
        output_name.clear();

      //todo: validate parameters;
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


int main( int argc, char* argv[] )
{
  int return_value = EXIT_SUCCESS;
//  try
    {
      application_parameters params( argc, argv );
      for( auto& filename : params.input_filename )
        {
          graphics_origin::geometry::mesh input(filename);
          median_path::median_skeleton result;

          median_path::skeletonizer algorithm( input, result, params.skeletonizer_parameters );

          result.save( params.get_output_filename( filename ) );

          std::cout<<"skeletonization of a " << input.n_vertices() << " vertices / " << input.n_faces()
              << " faces mesh to produce a median skeleton of " << result.get_number_of_atoms() << " atoms in "
              << algorithm.get_execution_time() << " second(s)" << std::endl;
        }
    }
//  catch( std::exception& e )
//    {
//      LOG( fatal, "exception caught: " << e.what() );
//      std::cerr << "exception caught: " << e.what() << std::endl;
//      return_value = EXIT_FAILURE;
//    }
  boost::log::core::get()->flush();
  return return_value;
}
