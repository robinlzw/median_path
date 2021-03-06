/*  Created on: Mar 31, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# ifndef MEDIAN_PATH_SIMPLE_GL_WINDOW_H_
# define MEDIAN_PATH_SIMPLE_GL_WINDOW_H_
# include <graphics-origin/application/window.h>
# include "../common/skeletons_renderable.h"
# include "../../median-path/skeletonization.h"

class benchmark_viewer_window
  : public graphics_origin::application::window {
  Q_OBJECT
  Q_PROPERTY( bool has_voronoi_geometry READ get_has_voronoi_geometry NOTIFY has_voronoi_geometry_changed )
  Q_PROPERTY( bool has_polar_geometry READ get_has_polar_geometry NOTIFY has_polar_geometry_changed )
  Q_PROPERTY( bool has_shrinking_geometry READ get_has_shrinking_geometry NOTIFY has_shrinking_geometry_changed )

  Q_PROPERTY( bool voronoi_geometry_active READ get_voronoi_geometry_active NOTIFY voronoi_geometry_active_changed )
  Q_PROPERTY( bool polar_geometry_active READ get_polar_geometry_active NOTIFY polar_geometry_active_changed )
  Q_PROPERTY( bool shrinking_geometry_active READ get_shrinking_geometry_active NOTIFY shrinking_geometry_active_changed )

  Q_PROPERTY( bool has_voronoi_reconstruction READ get_has_voronoi_reconstruction NOTIFY has_voronoi_reconstruction_changed )
  Q_PROPERTY( bool has_powershape_reconstruction READ get_has_powershape_reconstruction NOTIFY has_powershape_reconstruction_changed )
  Q_PROPERTY( bool has_delaunay_reconstruction READ get_has_delaunay_reconstruction NOTIFY has_delaunay_reconstruction_changed )
  Q_PROPERTY( bool has_weighted_alpha_reconstruction READ get_has_weighted_alpha_reconstruction NOTIFY has_weighted_alpha_reconstruction_changed )

  Q_PROPERTY( bool voronoi_reconstruction_active READ get_voronoi_reconstruction_active NOTIFY voronoi_reconstruction_active_changed )
  Q_PROPERTY( bool powershape_reconstruction_active READ get_powershape_reconstruction_active NOTIFY powershape_reconstruction_active_changed )
  Q_PROPERTY( bool delaunay_reconstruction_active READ get_delaunay_reconstruction_active NOTIFY delaunay_reconstruction_active_changed )
  Q_PROPERTY( bool weighted_alpha_reconstruction_active READ get_weighted_alpha_reconstruction_active NOTIFY weighted_alpha_reconstruction_active_changed )

public:
  benchmark_viewer_window( QQuickItem* parent = nullptr );

  /**
   * If the geometry method is available, those methods change the active geometry method.
   * The return value tells if the previous topology method is available for the new geometry method.
   * If this is not the case, the topology method should be changed too.
   */
  Q_INVOKABLE bool request_voronoi_geometry();
  Q_INVOKABLE bool request_polar_geometry();
  Q_INVOKABLE bool request_shrinking_geometry();


  /**
   * If the topology method is available for the current geometry method, those functions
   * change the active skeleton.
   */
  Q_INVOKABLE bool request_voronoi_reconstruction();
  Q_INVOKABLE bool request_powershape_reconstruction();
  Q_INVOKABLE bool request_delaunay_reconstruction();
  Q_INVOKABLE bool request_weighted_alpha_reconstruction();

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


  bool get_has_voronoi_geometry() const;
  bool get_has_polar_geometry() const;
  bool get_has_shrinking_geometry() const;

  bool get_voronoi_geometry_active() const;
  bool get_polar_geometry_active() const;
  bool get_shrinking_geometry_active() const;

  bool get_has_voronoi_reconstruction() const;
  bool get_has_powershape_reconstruction() const;
  bool get_has_delaunay_reconstruction() const;
  bool get_has_weighted_alpha_reconstruction() const;

  bool get_voronoi_reconstruction_active() const;
  bool get_powershape_reconstruction_active() const;
  bool get_delaunay_reconstruction_active() const;
  bool get_weighted_alpha_reconstruction_active() const;

signals:

  void has_voronoi_geometry_changed();
  void has_polar_geometry_changed();
  void has_shrinking_geometry_changed();

  void voronoi_geometry_active_changed();
  void polar_geometry_active_changed();
  void shrinking_geometry_active_changed();

  void has_voronoi_reconstruction_changed();
  void has_powershape_reconstruction_changed();
  void has_delaunay_reconstruction_changed();
  void has_weighted_alpha_reconstruction_changed();

  void voronoi_reconstruction_active_changed();
  void powershape_reconstruction_active_changed();
  void delaunay_reconstruction_active_changed();
  void weighted_alpha_reconstruction_active_changed();

private:
  void emit_active_geometry_method_has_changed();
  void emit_active_topology_method_has_changed();
  void emit_available_geometry_methods_has_changed();
  void emit_available_topology_methods_has_changed();

  static constexpr int number_of_geometries = 3;
  static constexpr int number_of_topologies = 4;

  median_path::median_skeletons_renderable::handle m_handles[ number_of_geometries ][ number_of_topologies ];
  median_path::median_skeletons_renderable* m_skeletons;
  typedef median_path::skeletonizer::parameters::geometry_method geometry_method;
  typedef median_path::structurer::parameters::topology_method topology_method;

  geometry_method m_geometry;
  topology_method m_topology;
};

# endif
