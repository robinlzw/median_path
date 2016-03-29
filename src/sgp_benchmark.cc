/*  Created on: Mar 28, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "../median-path/skeletonization.h"

# include <graphics-origin/tools/log.h>

# include <boost/program_options.hpp>
# include <boost/filesystem.hpp>

# include <sys/ioctl.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <iostream>

static const std::string version_string =
  "SGP 2016 Benchmark Building command line tool v0.1 ©2016 Thomas Delame";

static const std::string help_string =
  "Build SGP 2016 Benchmark\n"\
  "========================\n\n"\
  "Build the necessary skeleton data for the SGP 2016 paper. "\
  "The results are stored in the following directories, relatively to the root directory:\n"\
  "  voronoi/ for Voronoi ball skeletonization\n"\
  "    voronoi/ (Voronoi diagram reconstruction)\n"\
  "    powershape/ (Powershape reconstruction)\n"\
  "    delaunay/ \n"\
  "    weighted_alpha_shape/ \n"
  "  powershape/ for polar ball skeletonization\n"\
  "    powershape/ (Powershape reconstruction)\n"\
  "    delaunay/ \n"\
  "    weighted_alpha_shape/ \n"
  "  shrinking_ball/ for shrinking ball skeletonization\n"\
  "    delaunay/ \n"\
  "    weighted_alpha_shape/ \n"
  ;

static uint32_t get_console_line_length()
{
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return w.ws_col;
}

namespace po = boost::program_options;

struct application_parameters {
  std::vector< std::string > input_mesh_names;
  std::string output_directory;
  std::string input_directory;
  std::string output_extension;

  application_parameters( int argc, char* argv[] )
  {
    auto line_length = get_console_line_length();
    po::options_description generic(
        "Generic options", line_length,
        line_length > 10 ? line_length - 10 : line_length );

    generic.add_options()
     ("version,v", "print version string")
     ("help,h", "produce help message")
     ("output_directory,d", po::value<std::string>(&output_directory),
         "skeleton output root directory (current directory if not specified)." )
     ("input_directory,i", po::value<std::string>(&input_directory),
         "when specified, all mesh files recursively found in this directory will be used to build skeletons")
     ("format,f", po::value<std::string>(&output_extension)->default_value("web"),
         "output skeleton files format (median, web).")
    ;

    po::options_description hidden("Hidden Options");
    hidden.add_options()
     ("input",po::value<std::vector<std::string>>(&input_mesh_names));
    po::positional_options_description p;
    p.add("input", -1);

    po::options_description cmdline_options;
    cmdline_options.add( generic ).add( hidden );

    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv ).options( cmdline_options ).positional(p).run(),
        vm);
    po::notify(vm);

    if( vm.count("version") )
      std::cout << version_string << std::endl;

    if( vm.count("help") )
      {
        std::cout<< help_string << generic << std::endl;
        exit( EXIT_SUCCESS );
      }

    for( auto iterator = input_mesh_names.begin(), end = input_mesh_names.end();
        iterator != end; )
      {
        if( graphics_origin::geometry::has_a_mesh_file_extension( *iterator ) )
          ++iterator;
        else
          {
            LOG( error, "file " << *iterator << " has not a known mesh file extension");
            iterator = input_mesh_names.erase( iterator );
          }
      }

    if( !input_directory.empty() )
      {
        boost::filesystem::recursive_directory_iterator itr(input_directory);
        while (itr != boost::filesystem::recursive_directory_iterator())
          {
            std::string filename = itr->path().string();
            if( graphics_origin::geometry::has_a_mesh_file_extension( filename ) )
              input_mesh_names.push_back( filename );
            ++itr;
          }
      }

    if( input_mesh_names.empty() )
      {
        LOG( error, "no input mesh file. Please specify some input file names or give an input directory");
        std::cout << help_string << generic << std::endl;
        exit( EXIT_FAILURE );
      }

    if( output_directory.empty() )
      {
        output_directory = boost::filesystem::current_path().string();
      }
  }

  std::string
  get_output_filename(
      const std::string& input_filename,
      median_path::skeletonizer::parameters::geometry_method geometry,
      median_path::structurer::parameters::topology_method topology )
  {
    static const std::string geometry_directories[3] = {
        "shrinking_ball",
        "powershape",
        "voronoi"
    };
    static const std::string topology_directories[3] = {
        "weighted_alpha_shape",
        "delaunay"
        "powershape",
        "voronoi"
    };

    return output_directory + '/' + geometry_directories[geometry]
                            + '/' + topology_directories[topology]
                            + '/' + graphics_origin::tools::get_stem( input_filename )
                            + '.' + output_extension;
  }
};

static void benchmark_voronoi_reconstruction(
    graphics_origin::geometry::mesh& input,
    const std::string& filename,
    application_parameters& app_params,
    median_path::skeletonizer::parameters& params )
{
  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::VORONOI;
  {
    median_path::median_skeleton result;
    median_path::skeletonizer algorithm( input, result, params );
    std::cout << result.get_number_of_atoms() << " & "
        << result.get_number_of_links() << " & "
        << result.get_number_of_faces() << " & "
        << algorithm.get_execution_time() << " & ";
    result.save(
        app_params.get_output_filename(
            filename,
            params.m_geometry_method,
            params.m_structurer_parameters.m_topology_method ) );
  }
}

static void benchmark_powershape_reconstruction(
    graphics_origin::geometry::mesh& input,
    const std::string& filename,
    application_parameters& app_params,
    median_path::skeletonizer::parameters& params )
{
  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::POWERSHAPE;
  {
    median_path::median_skeleton result;
    median_path::skeletonizer algorithm( input, result, params );
    std::cout << result.get_number_of_links() << " & "
        << result.get_number_of_faces() << " & "
        << algorithm.get_execution_time() << " & ";
    result.save(
        app_params.get_output_filename(
            filename,
            params.m_geometry_method,
            params.m_structurer_parameters.m_topology_method ) );
  }
}

static void benchmark_delaunay_reconstruction(
    graphics_origin::geometry::mesh& input,
    const std::string& filename,
    application_parameters& app_params,
    median_path::skeletonizer::parameters& params )
{
  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::DELAUNAY_RECONSTRUCTION;
  {
    median_path::median_skeleton result;
    median_path::skeletonizer algorithm( input, result, params );
    std::cout << result.get_number_of_links() << " & "
        << result.get_number_of_faces() << " & "
        << algorithm.get_execution_time() << " & ";
    result.save(
        app_params.get_output_filename(
            filename,
            params.m_geometry_method,
            params.m_structurer_parameters.m_topology_method ) );
  }
}

static void benchmark_weighted_alpha_shape_reconstruction(
    graphics_origin::geometry::mesh& input,
    const std::string& filename,
    application_parameters& app_params,
    median_path::skeletonizer::parameters& params )
{
  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::WEIGHTED_ALPHA_SHAPE;
  {
    median_path::median_skeleton result;
    median_path::skeletonizer algorithm( input, result, params );
    std::cout << result.get_number_of_links() << " & "
        << result.get_number_of_faces() << " & "
        << algorithm.get_execution_time() << " & ";
    result.save(
        app_params.get_output_filename(
            filename,
            params.m_geometry_method,
            params.m_structurer_parameters.m_topology_method ) );
  }
}


static void benchmark_from_voronoi_skeletonization(
    graphics_origin::geometry::mesh& input,
    const std::string& filename,
    application_parameters& app_params,
    median_path::skeletonizer::parameters& params )
{
  params.m_geometry_method = median_path::skeletonizer::parameters::VORONOI_BALLS;
  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::VORONOI;
  benchmark_voronoi_reconstruction( input, filename, app_params, params );

  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::POWERSHAPE;
  benchmark_powershape_reconstruction( input, filename, app_params, params );

  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::DELAUNAY_RECONSTRUCTION;
  benchmark_delaunay_reconstruction( input, filename, app_params, params );

  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::WEIGHTED_ALPHA_SHAPE;
  benchmark_weighted_alpha_shape_reconstruction( input, filename, app_params, params );
}

static void benchmark_from_powershape_skeletonization(
    graphics_origin::geometry::mesh& input,
    const std::string& filename,
    application_parameters& app_params,
    median_path::skeletonizer::parameters& params )
{
  params.m_geometry_method = median_path::skeletonizer::parameters::POLAR_BALLS;
  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::POWERSHAPE;
  benchmark_powershape_reconstruction( input, filename, app_params, params );

  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::DELAUNAY_RECONSTRUCTION;
  benchmark_delaunay_reconstruction( input, filename, app_params, params );

  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::WEIGHTED_ALPHA_SHAPE;
  benchmark_weighted_alpha_shape_reconstruction( input, filename, app_params, params );
}

static void benchmark_from_shrinking_ball_skeletonization(
    graphics_origin::geometry::mesh& input,
    const std::string& filename,
    application_parameters& app_params,
    median_path::skeletonizer::parameters& params )
{
  params.m_geometry_method = median_path::skeletonizer::parameters::SHRINKING_BALLS;
  params.m_shrinking_ball.m_radius_method = median_path::skeletonizer::shrinking_ball_parameters::CONSTANT;
  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::DELAUNAY_RECONSTRUCTION;
  benchmark_delaunay_reconstruction( input, filename, app_params, params );

  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::WEIGHTED_ALPHA_SHAPE;
  benchmark_weighted_alpha_shape_reconstruction( input, filename, app_params, params );
}

static void prepare_directories( application_parameters& params )
{
  boost::filesystem::create_directories( params.output_directory + "/voronoi/voronoi");
  boost::filesystem::create_directories( params.output_directory + "/voronoi/powershape");
  boost::filesystem::create_directories( params.output_directory + "/voronoi/delaunay");
  boost::filesystem::create_directories( params.output_directory + "/voronoi/weighted_alpha_shape");

  boost::filesystem::create_directories( params.output_directory + "/powershape/powershape");
  boost::filesystem::create_directories( params.output_directory + "/powershape/delaunay");
  boost::filesystem::create_directories( params.output_directory + "/powershape/weighted_alpha_shape");

  boost::filesystem::create_directories( params.output_directory + "/shrinking_ball/delaunay");
  boost::filesystem::create_directories( params.output_directory + "/shrinking_ball/weighted_alpha_shape");
}

int main( int argc, char* argv[] )
{
  int return_value = EXIT_SUCCESS;
  try
    {
      application_parameters params( argc, argv );

      prepare_directories( params );



      median_path::skeletonizer::parameters skeletonizer_params;
      skeletonizer_params.m_build_topology = true;
      skeletonizer_params.m_merge_clusters = false;
      skeletonizer_params.m_structurer_parameters.m_build_faces = true;
      skeletonizer_params.m_structurer_parameters.m_neighbors_should_intersect = true;

      for( auto& filename : params.input_mesh_names )
        {
          std::cout << graphics_origin::tools::get_basename( filename ) << " & ";
          graphics_origin::geometry::mesh input(filename);

          benchmark_from_voronoi_skeletonization( input, filename, params, skeletonizer_params );
          benchmark_from_powershape_skeletonization( input, filename, params, skeletonizer_params );
          benchmark_from_shrinking_ball_skeletonization( input, filename, params, skeletonizer_params );
          std::cout << std::endl;
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
