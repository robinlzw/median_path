/*  Created on: Mar 29, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */

# ifndef MEDIAN_PATH_SIMPLE_CAMERA_H_
# define MEDIAN_PATH_SIMPLE_CAMERA_H_
# include "../median-path/median_path.h"
# include <graphics-origin/application/camera.h>

namespace median_path {

  class simple_camera
    : public graphics_origin::application::camera {
    Q_OBJECT
  public:
    explicit simple_camera( QObject* parent = nullptr );

    Q_INVOKABLE void set_go_left( bool left );
    Q_INVOKABLE void set_go_right( bool right );
    Q_INVOKABLE void set_go_forward( bool forward);
    Q_INVOKABLE void set_go_backward( bool backward );
    Q_INVOKABLE void set_go_up( bool up );
    Q_INVOKABLE void set_go_down( bool down );
    Q_INVOKABLE void mouse_rotation( qreal dx, qreal dy );

  private:
    void do_update() override;

    gpu_vec3 m_direction;
    real m_last_update_time;
    struct move {
      move();
      uint8_t forward : 1;
      uint8_t backward: 1;
      uint8_t left    : 1;
      uint8_t right   : 1;
      uint8_t up      : 1;
      uint8_t down    : 1;
    };
    move m_move;
    gpu_real m_mouse_dx;
    gpu_real m_mouse_dy;
    bool m_mouse_moved;
  };

}
# endif
