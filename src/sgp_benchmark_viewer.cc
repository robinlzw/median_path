/*  Created on: Mar 29, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */

# include "full_screen_scene_application.h"
# include "simple_camera.h"
# include "simple_gl_renderer.h"
# include "skeletons_renderable.h"

# include <graphics-origin/application/gl_window.h>
# include <graphics-origin/application/shader_program.h>

# include <QGuiApplication>

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
    auto handle = skeletons->add( "ico.median" );
    auto& storage = skeletons->get( handle );
    storage.active = true;
    add_renderable( skeletons );

    //TODO: initialize shader programs and renderables depending on the application arguments
  }
};


int main( int argc, char* argv[] )
{
  int dummy_argc = 1;
  QGuiApplication qgui( dummy_argc, argv );

  qmlRegisterType<simple_gl_window>("MedianPath", 1, 0, "GLWindow");
  qmlRegisterType<median_path::simple_camera>("MedianPath", 1, 0, "GLCamera");

  median_path::full_screen_scene_application app;
  app.show();
  app.raise();
  return qgui.exec();
}
