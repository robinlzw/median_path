/*  Created on: Mar 31, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# ifndef MEDIAN_PATH_SIMPLE_GL_WINDOW_H_
# define MEDIAN_PATH_SIMPLE_GL_WINDOW_H_
# include <graphics-origin/application/gl_window.h>
# include "skeletons_renderable.h"
# include "../median-path/skeletonization.h"

class viewer_window
  : public graphics_origin::application::gl_window {
  Q_OBJECT

public:
  viewer_window( QQuickItem* parent = nullptr );

  Q_INVOKABLE void render_balls( bool render );
  Q_INVOKABLE void render_skeleton_points( bool render );
  Q_INVOKABLE void render_triangles( bool render );
  Q_INVOKABLE void render_isolated_atoms( bool render );
  Q_INVOKABLE void render_isolated_links( bool render );
  Q_INVOKABLE void render_border_junction_links( bool render );
  Q_INVOKABLE void render_wireframe( bool render );
  Q_INVOKABLE void use_radii_colors( bool use );

  Q_INVOKABLE void save_camera(const QString& filename );
  Q_INVOKABLE void load_camera( const QString& filename );
  Q_INVOKABLE void reset_camera();
  Q_INVOKABLE void set_atom_color( const QColor& color );
  Q_INVOKABLE void set_isolated_color( const QColor& color );

  //fixme: put the viewer parameters in a header and send an instance to this function
  void load_benchmark(
      const std::string& shape_stem,
      const std::string& benchmark_directory,
      const std::string& extension,
      std::vector< median_path::skeletonizer::parameters::geometry_method>& geometries,
      std::vector< median_path::structurer::parameters::topology_method>& topologies );


signals:


private:
  median_path::median_skeletons_renderable* m_skeletons;
};

# endif
