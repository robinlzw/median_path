/*  Created on: Mar 31, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */

# ifndef MEDIAN_PATH_SIMPLE_GL_WINDOW_H_
# define MEDIAN_PATH_SIMPLE_GL_WINDOW_H_
# include <graphics-origin/application/gl_window.h>
# include "skeletons_renderable.h"

class simple_gl_window
  : public graphics_origin::application::gl_window {
  Q_OBJECT
public:
  simple_gl_window( QQuickItem* parent = nullptr );
  Q_INVOKABLE void request_voronoi_geometry();
  Q_INVOKABLE void request_polar_geometry();
  Q_INVOKABLE void request_shrinking_geometry();

  Q_INVOKABLE void request_voronoi_reconstruction();
  Q_INVOKABLE void request_powershape_reconstruction();
  Q_INVOKABLE void request_delaunay_reconstruction();
  Q_INVOKABLE void request_weighted_alpha_reconstruction();

  Q_INVOKABLE void render_isolated_atoms( bool render );
  Q_INVOKABLE void render_isolated_links( bool render );

  void load_benchmark( const std::string& shape_stem, const std::string& benchmark_directory, const std::string& extension );

private:
  enum{ voronoi_geometry, polar_geometry, shrinking_geometry, number_of_geometries };
  enum{ delaunay_reconstruction, weighted_alpha_reconstruction,
    powershape_reconstruction, voronoi_reconstruction, number_of_reconstructions};
  median_path::median_skeletons_renderable::handle m_handles[number_of_geometries][number_of_reconstructions];
  median_path::median_skeletons_renderable* m_skeletons;
  int m_geometry;
  int m_topology;
};

# endif
