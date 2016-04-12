/* Created on: Apr 12, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# include "../median-path/io.h"
# include "../median-path/regularization.h"
# include "../median-path/structuration.h"
# include <graphics-origin/tools/filesystem.h>
# include <graphics-origin/tools/log.h>

# include <boost/program_options.hpp>
# include <boost/filesystem.hpp>

# include <sys/ioctl.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <iostream>

static const std::string version_string =
    "Regularizer command line tool v0.1 ©2016 Thomas Delame";

static const std::string help_string =
    "Regularizer\n"\
    "===========\n"\
    "Regularize medial skeletons";

static uint32_t get_console_line_length()
{
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return w.ws_col;
}

namespace po = boost::program_options;

namespace std {

  std::istream& operator>>( std::istream& in, median_path::structurer::parameters::topology_method& method )
  {
    std::string token;
    in >> token;
    if( token == "weighted_alpha_shape" )
      method = median_path::structurer::parameters::WEIGHTED_ALPHA_SHAPE;
    else if( token == "delaunay" )
      method = median_path::structurer::parameters::DELAUNAY_RECONSTRUCTION;
    else if( token == "powershape")
      method = median_path::structurer::parameters::POWERSHAPE;
    else if( token == "voronoi" )
      method = median_path::structurer::parameters::VORONOI;
    else throw po::validation_error(
        po::validation_error::invalid_option_value,
        "topology_method",
        token);
    return in;
  }
}

struct application_parameters {
  std::vector< std::string > input_filename;
  std::string input_directory;
  std::string output_directory;
  std::string output_name;
  std::string output_extension;
  median_path::regularizer::parameters regularizer_parameters;
  median_path::structurer::parameters structurer_parameters;
  bool restructure;


  application_parameters( int argc, char* argv[] )
    : regularizer_parameters{},
      structurer_parameters{},
      restructure{ false }
  {
    auto line_length = get_console_line_length();
    po::options_description generic(
        "Generic options", line_length, line_length > 10 ? line_length -10 : line_length );
    generic.add_options()
        ("version,v", "print version string")
        ("help,h", "produce help message")
        ("input_directory,i", po::value<std::string>(&input_directory),
            "when specified, all skeleton files recursively found in this directory will be regularized")
        ("output_directory,d", po::value<std::string>(&output_directory), "skeleton output directory")
        ("output_name,o", po::value<std::string>(&output_name),"output name of the skeleton file when there is only one input skeleton")
        ("format,f", po::value<std::string>(&output_extension)->default_value("median"),"output file format (median, balls, moff and web)");

    po::options_description regularization(
        "Regularization options", line_length, line_length > 10 ? line_length -10 : line_length );
    regularization.add_options()
        ("scale,s", po::value<median_path::real>(&regularizer_parameters.m_scale_factor)->default_value(1.1),
            "scale factor for the scale regularizer")
        ("restructure,r", po::value<bool>(&restructure)->default_value(false),
            "switch on/off the restructuration after the regularization");

    po::options_description structuration(
        "Structuration options", line_length, line_length > 10 ? line_length -10 : line_length );
    structuration.add_options()
        ("topology_method,t",
            po::value<median_path::structurer::parameters::topology_method>(&structurer_parameters.m_topology_method)
            ->default_value(median_path::structurer::parameters::WEIGHTED_ALPHA_SHAPE),
            "method to build the skeleton topology. Possible values are:\n"
            "  * weighted_alpha_shape, to use a regular triangulation of atoms and keep only faces of the weighted alpha shape for alpha=0 (fast)\n"
            "  * delaunay_reconstruction, to use Delaunay reconstruction method\n"
            "  * powershape, to use the method of the PowerShape (long)\n"
            "  * voronoi, to use a subset of the voronoi faces/edges (fast since the Voronoi diagram is already available)")
        ("neighbors_intersect", po::value<bool>(&structurer_parameters.m_neighbors_should_intersect)->default_value(true),
            "switch on/off the intersection test between two neighbor atom candidates")
        ("build_faces", po::value<bool>(&structurer_parameters.m_build_faces)->default_value(true),
            "switch on/off the construction of triangular faces between atom centers");

      po::options_description visible;
      visible.add(generic).add(regularization).add(structuration);

      po::options_description hidden( "Hidden Options");
      hidden.add_options()("input", po::value<std::vector<std::string>>(&input_filename), "input skeleton file names");

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

      for( auto iterator = input_filename.begin(), end = input_filename.end();
          iterator != end; )
        {
          if( median_path::io::can_load_from( *iterator ) )
            ++iterator;
          else
            {
              LOG( error, "file " << *iterator << " is not a known skeleton file");
              iterator = input_filename.erase( iterator );
            }
        }
      if( !input_directory.empty() )
        {
          boost::filesystem::recursive_directory_iterator itr(input_directory);
          while (itr != boost::filesystem::recursive_directory_iterator())
            {
              std::string filename = itr->path().string();
              if( median_path::io::can_load_from( filename ) )
                input_filename.push_back( filename );
              ++itr;
            }
        }

      if( input_filename.empty() )
        {
          LOG( error, "no input skeleton file names");
          std::cout << help_string << visible << std::endl;
          exit( EXIT_FAILURE );
        }
      if( input_filename.size() > 1 )
        output_name.clear();
      if( output_directory.empty() )
        {
          output_directory = boost::filesystem::current_path().string();
        }

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
  try
    {
      application_parameters params( argc, argv );
      boost::filesystem::create_directories( params.output_directory );
      for( auto& filename : params.input_filename )
        {
          median_path::median_skeleton result(filename);
          auto natoms = result.get_number_of_atoms();
          auto nlinks = result.get_number_of_links();
          auto nfaces = result.get_number_of_faces();

          median_path::regularizer algorithm( result, params.regularizer_parameters );
          std::cout<<"regularization of a [" << natoms << ", " << nlinks << ", " << nfaces << "] skeleton into a ["
              << result.get_number_of_atoms() << ", " << result.get_number_of_links() << ", " << result.get_number_of_faces()
              << "] skeleton done in " << algorithm.get_execution_time() << " second(s)" << std::endl;

          if( params.restructure )
            {
              median_path::structurer structurer( result, params.structurer_parameters );
              std::cout << "structuration into a ["
                  << result.get_number_of_atoms() << ", " << result.get_number_of_links() << ", " << result.get_number_of_faces()
                  << "] skeleton done in " << structurer.get_execution_time() << " second(s)" << std::endl;
            }
          result.save( params.get_output_filename( filename ) );
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
