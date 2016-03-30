/* Created on: Mar 30, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# include "../median-path/median_path.h"
# include <graphics-origin/tools/log.h>
# include <graphics-origin/geometry/mesh.h>

# include <boost/program_options.hpp>
# include <boost/filesystem.hpp>

# include <sys/ioctl.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <iostream>
# include <iomanip>

static const std::string version_string =
  "SGP 2016 Benchmark Building command line tools v0.1 ©2016 Thomas Delame";

static const std::string help_string =
  "Prepare mesh for benchmark\n"\
  "==========================\n\n"\
  "In order to avoid many problems due to the assumption in the algorithms, a input mesh must verify some properties before being used in the benchmark. "\
  "This program check if a mesh is valid and resize its geometry to be contained in the unit sphere.\n"\
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
  std::string input_directory;
  std::string output_directory;
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
         "mesh output root directory (current directory if not specified)." )
     ("input_directory,i", po::value<std::string>(&input_directory),
         "when specified, all mesh files recursively found in this directory will be tested and prepared")
     ("format,f", po::value<std::string>(&output_extension)->default_value("ply"),
         "output mesh files format (ply, off).")
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
      const std::string& input_filename )
  {
    return output_directory + '/' + graphics_origin::tools::get_stem( input_filename )
                            + '.' + output_extension;
  }
};

int main( int argc, char* argv[] )
{
  int return_value = EXIT_SUCCESS;
  try
    {
      application_parameters params( argc, argv );
      boost::filesystem::create_directories( params.output_directory );
      for( auto& filename : params.input_mesh_names )
        {
          try
            {
              graphics_origin::geometry::mesh input(filename);
              bool ok = true;
              if( !input.n_vertices() || !input.n_faces() )
                {
                  LOG( info, "mesh " << filename << " does not have enough elements");
                  ok = false;
                }

              if( ok )
              for( auto vit = input.vertices_begin(), end = input.vertices_end();
                  vit != end; ++ vit )
                {
                  if( !input.is_manifold( *vit ) )
                    {
                      LOG( info, "mesh " << filename << " is not valid because it is not manifold");
                      ok = false;
                      break;
                    }
                  else if( input.is_boundary( *vit ) )
                    {
                      LOG( info, "mesh " << filename << " is not valid because it contains holes");
                      ok = false;
                      break;
                    }
                }

              if( ok )
                {
                  graphics_origin::geometry::aabox box;
                  input.compute_bounding_box( box );
                  median_path::real scale_factor = median_path::real(1.0) / std::max( box.m_hsides.x, std::max( box.m_hsides.y, box.m_hsides.z ) );
                  for( auto vit = input.vertices_begin(), end = input.vertices_end();
                      vit != end; ++ vit )
                    {
                      auto& p = input.point( *vit );
                      p[0] = (p[0] - box.m_center.x ) * scale_factor;
                      p[1] = (p[1] - box.m_center.y ) * scale_factor;
                      p[2] = (p[2] - box.m_center.z ) * scale_factor;
                    }

                  for( auto fit = input.faces_begin(), end = input.faces_end();
                      fit != end; ++ fit )
                    {
                      auto fvit = input.fv_begin( *fit );
                      auto& p1 = input.point( *fvit ); ++ fvit;
                      auto& p2 = input.point( *fvit ); ++ fvit;
                      auto& p3 = input.point( *fvit );

                      median_path::vec3 e1{ p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2] };
                      median_path::vec3 e2{ p3[0] - p1[0], p3[1] - p1[1], p3[2] - p1[2] };
                      median_path::real area = median_path::real(0.5) * median_path::length( median_path::cross( e1, e2 ) );
                      if( area < 5e-6 )
                        {
                          LOG( info, "mesh " << filename << " has triangle #" << fvit->idx() << " with too small area (" << area << ")");
                          ok = false;
                          break;
                        }
                    }
                }

              if( ok )
                {
                  input.save( params.get_output_filename(filename) );
                  LOG( info, "mesh " << filename << " is valid for the benchmark and has been prepared");
                }
            }
          catch( ... )
            {
              LOG( error, "something wrong happen while processing " << filename << ". It is likely to indicate that the mesh is not two-manifold");
            }
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
