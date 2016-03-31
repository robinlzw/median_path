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
  "  shapes/ to store the input shapes\n"\
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
    static const std::string topology_directories[4] = {
        "weighted_alpha_shape",
        "delaunay",
        "powershape",
        "voronoi"
    };

    return output_directory + '/' + geometry_directories[geometry]
                            + '/' + topology_directories[topology]
                            + '/' + graphics_origin::tools::get_stem( input_filename )
                            + '.' + output_extension;
  }
};

struct skeletonization_statistics {

  skeletonization_statistics()
    : atoms{0}, links{0}, faces{0},
      execution_time{0}
  {}

  uint32_t atoms;
  uint64_t links;
  uint64_t faces;
  median_path::real execution_time;
};

struct statistics {
  std::string mesh_name;
  uint32_t mesh_vertices;
  uint64_t mesh_links;
  uint64_t mesh_faces;

  skeletonization_statistics voronoi_voronoi;
  skeletonization_statistics voronoi_powershape;
  skeletonization_statistics voronoi_delaunay;
  skeletonization_statistics voronoi_alpha;

  skeletonization_statistics powershape_powershape;
  skeletonization_statistics powershape_delaunay;
  skeletonization_statistics powershape_alpha;

  skeletonization_statistics shrinking_ball_delaunay;
  skeletonization_statistics shrinking_ball_alpha;
};

static void benchmark_voronoi_reconstruction(
    graphics_origin::geometry::mesh& input,
    const std::string& filename,
    application_parameters& app_params,
    median_path::skeletonizer::parameters& params,
    skeletonization_statistics& stats )
{
  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::VORONOI;
  median_path::median_skeleton result;
  median_path::skeletonizer algorithm( input, result, params );
  stats.atoms = result.get_number_of_atoms();
  stats.links = result.get_number_of_links();
  stats.faces = result.get_number_of_faces();
  stats.execution_time = algorithm.get_execution_time();
  result.save(
      app_params.get_output_filename(
          filename,
          params.m_geometry_method,
          params.m_structurer_parameters.m_topology_method ) );
}

static void benchmark_powershape_reconstruction(
    graphics_origin::geometry::mesh& input,
    const std::string& filename,
    application_parameters& app_params,
    median_path::skeletonizer::parameters& params,
    skeletonization_statistics& stats )
{
  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::POWERSHAPE;
  median_path::median_skeleton result;
  median_path::skeletonizer algorithm( input, result, params );
  stats.atoms = result.get_number_of_atoms();
  stats.links = result.get_number_of_links();
  stats.faces = result.get_number_of_faces();
  stats.execution_time = algorithm.get_execution_time();
  result.save(
      app_params.get_output_filename(
          filename,
          params.m_geometry_method,
          params.m_structurer_parameters.m_topology_method ) );
}

static void benchmark_delaunay_reconstruction(
    graphics_origin::geometry::mesh& input,
    const std::string& filename,
    application_parameters& app_params,
    median_path::skeletonizer::parameters& params,
    skeletonization_statistics& stats )
{
  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::DELAUNAY_RECONSTRUCTION;
  median_path::median_skeleton result;
  median_path::skeletonizer algorithm( input, result, params );
  stats.atoms = result.get_number_of_atoms();
  stats.links = result.get_number_of_links();
  stats.faces = result.get_number_of_faces();
  stats.execution_time = algorithm.get_execution_time();
  result.save(
      app_params.get_output_filename(
          filename,
          params.m_geometry_method,
          params.m_structurer_parameters.m_topology_method ) );
}

static void benchmark_weighted_alpha_shape_reconstruction(
    graphics_origin::geometry::mesh& input,
    const std::string& filename,
    application_parameters& app_params,
    median_path::skeletonizer::parameters& params,
    skeletonization_statistics& stats )
{
  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::WEIGHTED_ALPHA_SHAPE;
  median_path::median_skeleton result;
  median_path::skeletonizer algorithm( input, result, params );
  stats.atoms = result.get_number_of_atoms();
  stats.links = result.get_number_of_links();
  stats.faces = result.get_number_of_faces();
  stats.execution_time = algorithm.get_execution_time();
  result.save(
      app_params.get_output_filename(
          filename,
          params.m_geometry_method,
          params.m_structurer_parameters.m_topology_method ) );
}


static void benchmark_from_voronoi_skeletonization(
    graphics_origin::geometry::mesh& input,
    const std::string& filename,
    application_parameters& app_params,
    median_path::skeletonizer::parameters& params,
    statistics& stats )
{
  params.m_geometry_method = median_path::skeletonizer::parameters::VORONOI_BALLS;

  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::DELAUNAY_RECONSTRUCTION;
  benchmark_delaunay_reconstruction( input, filename, app_params, params, stats.voronoi_delaunay );

  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::VORONOI;
  benchmark_voronoi_reconstruction( input, filename, app_params, params, stats.voronoi_voronoi );

  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::POWERSHAPE;
  benchmark_powershape_reconstruction( input, filename, app_params, params, stats.voronoi_powershape );

  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::WEIGHTED_ALPHA_SHAPE;
  benchmark_weighted_alpha_shape_reconstruction( input, filename, app_params, params, stats.voronoi_alpha );
}

static void benchmark_from_powershape_skeletonization(
    graphics_origin::geometry::mesh& input,
    const std::string& filename,
    application_parameters& app_params,
    median_path::skeletonizer::parameters& params,
    statistics& stats )
{
  params.m_geometry_method = median_path::skeletonizer::parameters::POLAR_BALLS;

  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::DELAUNAY_RECONSTRUCTION;
  benchmark_delaunay_reconstruction( input, filename, app_params, params, stats.powershape_delaunay );

  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::POWERSHAPE;
  benchmark_powershape_reconstruction( input, filename, app_params, params, stats.powershape_powershape );

  params.m_structurer_parameters.m_topology_method = median_path::structurer::parameters::WEIGHTED_ALPHA_SHAPE;
  benchmark_weighted_alpha_shape_reconstruction( input, filename, app_params, params, stats.powershape_alpha );
}

static void benchmark_from_shrinking_ball_skeletonization(
    graphics_origin::geometry::mesh& input,
    const std::string& filename,
    application_parameters& app_params,
    median_path::skeletonizer::parameters& params,
    statistics& stats )
{
  params.m_geometry_method = median_path::skeletonizer::parameters::SHRINKING_BALLS;
  params.m_shrinking_ball.m_radius_method = median_path::skeletonizer::shrinking_ball_parameters::CONSTANT;
  benchmark_delaunay_reconstruction( input, filename, app_params, params, stats.shrinking_ball_delaunay );
  benchmark_weighted_alpha_shape_reconstruction( input, filename, app_params, params, stats.shrinking_ball_alpha );
}

static void prepare_directories( application_parameters& params )
{
  boost::filesystem::create_directories( params.output_directory + "/shapes");

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

      std::vector< statistics > stats;

      for( auto& filename : params.input_mesh_names )
        {
          graphics_origin::geometry::mesh input(filename);

          LOG( debug, "dealing now with input shape " << filename );

          statistics stat;
          stat.mesh_name = graphics_origin::tools::get_stem( filename );
          stat.mesh_vertices = input.n_vertices();
          stat.mesh_links = input.n_edges();
          stat.mesh_faces = input.n_faces();

          benchmark_from_voronoi_skeletonization( input, filename, params, skeletonizer_params, stat );
          benchmark_from_powershape_skeletonization( input, filename, params, skeletonizer_params, stat );
          benchmark_from_shrinking_ball_skeletonization( input, filename, params, skeletonizer_params, stat );

          stats.push_back( stat );

          input.save( params.output_directory + "/shapes/" + stat.mesh_name + ".ply");
        }

      std::cout << "Execution statistics:\n";
      for( auto& stat : stats )
        {
          std::cout << "for input mesh [" << stat.mesh_name << "]\n"
              << "* voronoi    - voronoi    in " << stat.voronoi_voronoi.execution_time << " (s)\n"
              << "    atoms " << stat.voronoi_voronoi.atoms << "\n"
              << "    links " << stat.voronoi_voronoi.links << "\n"
              << "    faces " << stat.voronoi_voronoi.faces << "\n"
              << "  voronoi    - powershape in " << stat.voronoi_powershape.execution_time << " (s)\n"
              << "    atoms " << stat.voronoi_powershape.atoms << "\n"
              << "    links " << stat.voronoi_powershape.links << "\n"
              << "    faces " << stat.voronoi_powershape.faces << "\n"
              << "  voronoi    - delaunay   in " << stat.voronoi_delaunay.execution_time << " (s)\n"
              << "    atoms " << stat.voronoi_delaunay.atoms << "\n"
              << "    links " << stat.voronoi_delaunay.links << "\n"
              << "    faces " << stat.voronoi_delaunay.faces << "\n"
              << "  voronoi    - alpha      in " << stat.voronoi_alpha.execution_time << " (s)\n"
              << "    atoms " << stat.voronoi_alpha.atoms << "\n"
              << "    links " << stat.voronoi_alpha.links << "\n"
              << "    faces " << stat.voronoi_alpha.faces << "\n"

              << "* powershape - powershape in " << stat.powershape_powershape.execution_time << " (s)\n"
              << "    atoms " << stat.powershape_powershape.atoms << "\n"
              << "    links " << stat.powershape_powershape.links << "\n"
              << "    faces " << stat.powershape_powershape.faces << "\n"
              << "  powershape - delaunay   in " << stat.powershape_delaunay.execution_time << " (s)\n"
              << "    atoms " << stat.powershape_delaunay.atoms << "\n"
              << "    links " << stat.powershape_delaunay.links << "\n"
              << "    faces " << stat.powershape_delaunay.faces << "\n"
              << "  powershape - alpha      in " << stat.powershape_alpha.execution_time << " (s)\n"
              << "    atoms " << stat.powershape_alpha.atoms << "\n"
              << "    links " << stat.powershape_alpha.links << "\n"
              << "    faces " << stat.powershape_alpha.faces << "\n"

              << "* shrinking  - delaunay   in " << stat.shrinking_ball_delaunay.execution_time << " (s)\n"
              << "    atoms " << stat.shrinking_ball_delaunay.atoms << "\n"
              << "    links " << stat.shrinking_ball_delaunay.links << "\n"
              << "    faces " << stat.shrinking_ball_delaunay.faces << "\n"
              << "  shrinking  - alpha      in " << stat.shrinking_ball_alpha.execution_time << " (s)\n"
              << "    atoms " << stat.shrinking_ball_alpha.atoms << "\n"
              << "    links " << stat.shrinking_ball_alpha.links << "\n"
              << "    faces " << stat.shrinking_ball_alpha.faces << "\n";
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
