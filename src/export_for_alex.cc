/*  Created on: Apr 4, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "../median-path/median_skeleton.h"
# include "../median-path/io.h"
# include <graphics-origin/tools/filesystem.h>
# include <graphics-origin/tools/log.h>

# include <boost/program_options.hpp>
# include <boost/filesystem.hpp>

# include <sys/ioctl.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <iostream>
# include <iomanip>

static const std::string version_string =
    "TODO v0.1 ©2016 Thomas Delame";

static const std::string help_string = "TODO";

static uint32_t get_console_line_length()
{
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return w.ws_col;
}

namespace po = boost::program_options;
struct application_parameters {
  std::vector< std::string > input_skeleton_names;
  std::string output_directory;
  std::string input_directory;

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
         "when specified, all skeleton files recursively found in this directory will be converted")
    ;

    po::options_description hidden("Hidden Options");
    hidden.add_options()
     ("input",po::value<std::vector<std::string>>(&input_skeleton_names));
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

    for( auto iterator = input_skeleton_names.begin(), end = input_skeleton_names.end();
        iterator != end; )
      {
        if( median_path::io::can_load_from( *iterator ) )
          ++iterator;
        else
          {
            LOG( error, "file " << *iterator << " has not a known skeleton file extension");
            iterator = input_skeleton_names.erase( iterator );
          }
      }

    if( !input_directory.empty() )
      {
        boost::filesystem::recursive_directory_iterator itr(input_directory);
        while (itr != boost::filesystem::recursive_directory_iterator())
          {
            std::string filename = itr->path().string();
            if( median_path::io::can_load_from( filename ) )
              input_skeleton_names.push_back( filename );
            ++itr;
          }
      }

    if( input_skeleton_names.empty() )
      {
        LOG( error, "no input skeleton file. Please specify some input file names or give an input directory");
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
                            + ".ply";
  }
};

int main( int argc, char* argv[] )
{
  int return_value = EXIT_SUCCESS;
  try
    {
      application_parameters params( argc, argv );
      boost::filesystem::create_directories( params.output_directory );
      for( auto& filename : params.input_skeleton_names )
        {
          median_path::median_skeleton s( filename );
          std::ofstream output( params.get_output_filename( filename ) );

          output << "ply\n"
              << "format ascii 1.0\n"
              << "element vertex " <<  s.get_number_of_atoms() << "\n"
              << "property float x\n"
              << "property float y\n"
              << "property float z\n"
              << "end_header\n";

          output.precision( 8 );
          s.process( [&output]( median_path::median_skeleton::atom& atom )
           {
              output << std::fixed << std::setw( 11 ) << atom.x
                  << " " << std::setw( 11 ) << atom.y
                  << " " << std::setw( 11 ) << atom.z << "\n";
           }, false );
          output.close();
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
