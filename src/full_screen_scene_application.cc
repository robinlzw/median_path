/*  Created on: Mar 29, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */

# include "full_screen_scene_application.h"

# include <graphics-origin/application/gl_window.h>

# include <QtGui/QSurfaceFormat>

namespace median_path {

  full_screen_scene_application::full_screen_scene_application( QWindow* parent )
    : QQuickView( parent )
  {
    QSurfaceFormat format;
    format.setMajorVersion( 4 );
    format.setMinorVersion( 4 );
    setFormat( format );
    setResizeMode(QQuickView::SizeRootObjectToView );
    // Rendering in a thread introduces a slightly more complicated cleanup
    // so we ensure that no cleanup of graphics resources happen until the
    // application is shutting down.
    setPersistentOpenGLContext(true);
    setPersistentSceneGraph(true);

    setSource(QUrl::fromLocalFile("qml/FullscreenSceneWindow.qml"));
  }

  full_screen_scene_application::~full_screen_scene_application()
  {
    // As the render threads make use of our QGuiApplication object
    // to clean up gracefully, wait for them to finish before
    // QGuiApp is taken off the heap.
    foreach(graphics_origin::application::gl_window* w, graphics_origin::application::gl_window::g_gl_windows)
      {
        w->pause();
        delete w;
      }
  }
}
