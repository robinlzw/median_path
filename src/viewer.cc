/* Created on: Apr 21, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# include "simple_qml_application.h"
# include "simple_camera.h"

# include <graphics-origin/tools/log.h>

# include <boost/program_options.hpp>
# include <boost/filesystem.hpp>

# include <sys/ioctl.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <iostream>

# include <QGuiApplication>
# include <QApplication>

static const std::string version_string =
  "Viewer tool v1.0 ©2016 Thomas Delame";

static const std::string help_string =
  "Viewer\n"\
  "======\n\n"\
  "Skeletons and Shapes viewer.\n"
  ;

static uint32_t get_console_line_length()
{
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return w.ws_col;
}

namespace po = boost::program_options;


struct application_parameters {
  std::string input_directory;
  std::vector< std::string > input_filenames;
  std::vector< std::string > skeleton_filenames;
  std::vector< std::string > mesh_filenames;
  bool recursive_traversal;

  bool process_filename( const std::string& filename  )
  {
    if( graphics_origin::geometry::has_a_mesh_file_extension( filename ) )
      {
        mesh_filenames.push_back( filename );
        return true;
      }
    else if( median_path::io::can_load_from( filename ) )
      {
        skeleton_filenames.push_back( filename );
        return true;
      }
    return false;
  }

  application_parameters( int argc, char* argv[] )
  {
    auto line_length = get_console_line_length();
    po::options_description generic(
        "Generic options", line_length,
        line_length > 10 ? line_length - 10 : line_length );
    po::options_description hidden("Hidden Options");
    po::positional_options_description positional;
    po::options_description cmdline_options;

    generic.add_options()
     ("version,v", "print version string")
     ("help,h", "produce help message")
     ("input_directory,d", po::value<std::string>(&input_directory),
         "load each mesh and skeleton found in this directory.")
     ("recursive,r", po::value<bool>(&recursive_traversal)->default_value(false),
         "if true, look recursively into the input directory for meshes and skeletons")
    ;

    hidden.add_options()
        ("input", po::value<std::vector<std::string>>(&input_filenames));

    positional.add("input",-1);

    cmdline_options.add( generic ).add( hidden );

    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv ).options( cmdline_options ).positional( positional ).run(),
        vm);
    po::notify(vm);

    if( vm.count("version") )
      std::cout << version_string << std::endl;

    if( vm.count("help") )
      {
        std::cout<< help_string << generic << std::endl;
        exit( EXIT_SUCCESS );
      }

    for( auto iterator = input_filenames.begin(), end = input_filenames.end();
        iterator != end; )
      {
        if( process_filename( *iterator ) )
          ++ iterator;
        else
          {
            LOG( error, "file " << *iterator << " is not a know mesh or skeleton file");
            iterator = input_filenames.erase( iterator );
          }
      }

    if( !input_directory.empty() )
      {
        if( recursive_traversal )
          {
            boost::filesystem::recursive_directory_iterator itr(input_directory);
            while (itr != boost::filesystem::recursive_directory_iterator())
              {
                process_filename( itr->path().string() );
                ++itr;
              }
          }
        else
          {
            boost::filesystem::directory_iterator itr( input_directory );
            while (itr != boost::filesystem::directory_iterator())
              {
                process_filename( itr->path().string() );
                ++itr;
              }
          }
      }

    if( mesh_filenames.empty() || skeleton_filenames.empty() )
      {
        LOG( error, "no input skeleton or mesh file. Please specify some input file names or give an input directory containing skeletons/meshes");
        std::cout << help_string << generic << std::endl;
        exit( EXIT_FAILURE );
      }
  }
};

int main( int argc, char* argv[] )
{
  application_parameters params = application_parameters( argc, argv );

  int dummy_argc = 1;
  QApplication qgui( dummy_argc, argv );

  qmlRegisterType<viewer_window>("MedianPath", 1, 0, "GLWindow");
  qmlRegisterType<median_path::simple_camera>("MedianPath", 1, 0, "GLCamera");

  median_path::simple_qml_application app;
  app.setSource( QUrl::fromLocalFile("qml/SGPViewer.qml"));
  app.show();

  auto window = app.rootObject()->findChild<viewe_windowr*>("glwindow");
  window->load_skeletons( params.skeleton_filenames );
  window->load_meshes( params.mesh_filenames );

  app.raise();
  return qgui.exec();
}
