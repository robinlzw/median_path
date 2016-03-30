/*  Created on: Mar 29, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "simple_camera.h"
# include <omp.h>

namespace median_path {
  simple_camera::move::move()
    : forward{0}, backward{0}, left{0}, right{0}, up{0}, down{0}
  {}

  simple_camera::simple_camera( QObject* parent )
    : graphics_origin::application::camera{ parent },
      m_last_update_time{ omp_get_wtime() },
      m_mouse_dx{0}, m_mouse_dy{0}, m_mouse_moved{ false }
  {}

  void simple_camera::set_go_left( bool left )
  {
    m_move.left = left;
  }
  void simple_camera::set_go_right( bool right )
  {
    m_move.right = right;
  }
  void simple_camera::set_go_forward( bool forward)
  {
    m_move.forward = forward;
  }
  void simple_camera::set_go_backward( bool backward )
  {
    m_move.backward = backward;
  }
  void simple_camera::set_go_up( bool up )
  {
    m_move.up = up;
  }
  void simple_camera::set_go_down( bool down )
  {
    m_move.down = down;
  }
  void simple_camera::mouse_rotation( qreal dx, qreal dy )
  {
    m_mouse_dx += dx;
    m_mouse_dy += dy;
    m_mouse_moved = true;
  }

  void simple_camera::do_update()
  {
    m_direction.x = m_move.left    - m_move.right;
    m_direction.y = m_move.up      - m_move.down;
    m_direction.z = m_move.forward - m_move.backward;
    gpu_real factor = dot( m_direction, m_direction );
    if( factor > 0.01 )
      {
        factor = gpu_real( (omp_get_wtime() - m_last_update_time) * 0.5 ) / std::sqrt( factor );
        m_direction *= factor;

        m_view[3][0] += m_direction.x;
        m_view[3][1] += m_direction.y;
        m_view[3][2] += m_direction.z;

        emit position_changed();
      }

    if( m_mouse_moved )
      {
        spaceship_rotate( m_mouse_dx, m_mouse_dy );

        m_mouse_dx = 0;
        m_mouse_dy = 0;
        m_mouse_moved = false;
      }
    m_last_update_time = omp_get_wtime();
  }
}
