/*  Created on: Mar 29, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */

# include "full_screen_scene_application.h"
# include "simple_camera.h"
# include "simple_gl_renderer.h"

# include <graphics-origin/application/gl_window.h>

# include <QGuiApplication>

class simple_gl_window
  : public graphics_origin::application::gl_window {
public:
  simple_gl_window( QQuickItem* parent = nullptr )
    : graphics_origin::application::gl_window( parent )
  {
    initialize_renderer( new median_path::simple_gl_renderer );

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
