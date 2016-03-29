/*  Created on: Mar 29, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */

# ifndef MEDIAN_PATH_FULL_SCREEN_SCENE_APPLICATION_H_
# define MEDIAN_PATH_FULL_SCREEN_SCENE_APPLICATION_H_
# include "../median-path/median_path.h"
# include <graphics-origin/application/gl_window_renderer.h>

# include <QtQuick/QQuickView>

namespace median_path {

  class full_screen_scene_application
    : public QQuickView {
    Q_OBJECT
  public:
    explicit full_screen_scene_application( QWindow* parent = nullptr );
    ~full_screen_scene_application();
  };


}
# endif
