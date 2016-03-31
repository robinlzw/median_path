/*  Created on: Mar 29, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "full_screen_scene_application.h"
# include "simple_camera.h"
# include "simple_gl_renderer.h"
# include "skeletons_renderable.h"

# include <graphics-origin/application/gl_window.h>
# include <graphics-origin/application/shader_program.h>
# include <graphics-origin/tools/log.h>

# include <boost/program_options.hpp>
# include <boost/filesystem.hpp>

# include <sys/ioctl.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <iostream>

# include <QGuiApplication>

static const std::string version_string =
  "SGP 2016 Benchmark Viewer tool v0.1 ©2016 Thomas Delame";

static const std::string help_string =
  "SGP 2016 Benchmark Viewer\n"\
  "=========================\n\n"\
  "Viewer of the benchmark results.\n"
  ;

static uint32_t get_console_line_length()
{
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return w.ws_col;
}

namespace po = boost::program_options;

struct application_parameters {
  std::string input_stem;
  std::string input_directory;

  application_parameters()
  {}


  application_parameters( int argc, char* argv[] )
  {
    auto line_length = get_console_line_length();
    po::options_description generic(
        "Generic options", line_length,
        line_length > 10 ? line_length - 10 : line_length );

    generic.add_options()
     ("version,v", "print version string")
     ("help,h", "produce help message")
     ("input_directory,d", po::value<std::string>(&input_directory),
         "where the benchmark results had been stored. If not specified, this is the working directory.")
     ("stem,s", po::value<std::string>(&input_stem),
         "stem of the input skeletons to compare (the basename of the input mesh used to compute the skeleton, from which we removed the extension part)")
    ;

    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv ).options( generic ).run(),
        vm);
    po::notify(vm);

    if( vm.count("version") )
      std::cout << version_string << std::endl;

    if( vm.count("help") )
      {
        std::cout<< help_string << generic << std::endl;
        exit( EXIT_SUCCESS );
      }

    if( input_directory.empty() )
      {
        boost::filesystem::current_path().string();
      }

    if( input_stem.empty() )
      {
        LOG( error, "no input stem. Please specify one input stem to launch the viewer");
        std::cout << help_string << generic << std::endl;
        exit( EXIT_FAILURE );
      }
  }
};


static application_parameters params;

class simple_gl_window
  : public graphics_origin::application::gl_window {
public:
  simple_gl_window( QQuickItem* parent = nullptr )
    : graphics_origin::application::gl_window( parent )
  {
    initialize_renderer( new median_path::simple_gl_renderer );
    auto skeleton_program = std::make_shared<graphics_origin::application::shader_program>( std::list<std::string>{
      "shaders/skeleton.vert",
      "shaders/skeleton.geom",
      "shaders/skeleton.frag"
    });


    auto skeletons = new median_path::median_skeletons_renderable( skeleton_program );
//    auto handle = skeletons->add( params.input_directory + "/voronoi/delaunay/" + params.input_stem + ".median" );
    auto handle = skeletons->add( "build/benchmark/voronoi/weighted_alpha_shape/ico.median" );//params.input_directory + "/voronoi/delaunay/" + params.input_stem + ".median" );
    auto& storage = skeletons->get( handle );
    storage.active = true;
    add_renderable( skeletons );

    //TODO: initialize shader programs and renderables depending on the application arguments
  }
};


int main( int argc, char* argv[] )
{
  params = application_parameters( argc, argv );

  int dummy_argc = 1;
  QGuiApplication qgui( dummy_argc, argv );

  qmlRegisterType<simple_gl_window>("MedianPath", 1, 0, "GLWindow");
  qmlRegisterType<median_path::simple_camera>("MedianPath", 1, 0, "GLCamera");

  median_path::full_screen_scene_application app;
  app.show();
  app.raise();
  return qgui.exec();
}
