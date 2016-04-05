/*  Created on: Mar 31, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "simple_gl_window.h"
# include "simple_gl_renderer.h"


# include <graphics-origin/application/shader_program.h>
# include <graphics-origin/tools/log.h>
# include <graphics-origin/application/camera.h>

# include <fstream>


  constexpr int simple_gl_window::number_of_geometries;
  constexpr int simple_gl_window::number_of_topologies;


  simple_gl_window::simple_gl_window( QQuickItem* parent )
    : graphics_origin::application::gl_window( parent ), m_skeletons{ nullptr }, m_geometry{geometry_method::VORONOI_BALLS}, m_topology{topology_method::DELAUNAY_RECONSTRUCTION}
  {
    initialize_renderer( new median_path::simple_gl_renderer );
  }

  void simple_gl_window::save_camera(const QString& filename )
  {
    std::string f = filename.toUtf8().constData();
    if( !f.empty() )
      {
        std::ofstream output( f );
        if( output.is_open() )
          {
            output << m_renderer->get_view_matrix();
            output.close();
            LOG( info, "camera view matrix saved to file [" << f << "]");
          }
        else
          {
            LOG( error, "cannot save to file [" << f << "]");
          }
      }
  }

  void simple_gl_window::load_camera( const QString& filename )
  {
    std::string f = filename.toUtf8().constData();
    if( !f.empty() )
      {
        std::ifstream input( f );
        if( input.is_open() )
          {
            median_path::gpu_mat4 view;
            input >> view;
            if( !input.fail() )
              {
                m_renderer->set_view_matrix( view );
                LOG( info, "camera view loaded from [" << f << "]");
              }
            else
              {
                LOG( info, "failed to read camera from [" << f << "]");
              }
            input.close();

          }
        else
          {
            LOG( error, "cannot open camera file [" << f << "]");
          }
      }
  }

  void simple_gl_window::reset_camera()
  {
    m_renderer->set_view_matrix( glm::lookAt( median_path::gpu_vec3{3,0,0}, median_path::gpu_vec3{}, median_path::gpu_vec3{0,0,1}) );
  }


  void
  simple_gl_window::load_benchmark(
      const std::string& shape_stem, const std::string& benchmark_directory, const std::string& extension,
      std::vector< median_path::skeletonizer::parameters::geometry_method>& geometries )
  {
    if( ! m_skeletons )
      {
        auto skeleton_program = std::make_shared<graphics_origin::application::shader_program>( std::list<std::string>{
          "shaders/skeleton.vert",
          "shaders/skeleton.geom",
          "shaders/skeleton.frag"
        });

        auto isolated_program = std::make_shared<graphics_origin::application::shader_program>( std::list<std::string>{
          "shaders/flat_skeleton.vert",
          "shaders/flat_skeleton.frag"
        });
        auto border_junction_program = std::make_shared<graphics_origin::application::shader_program>( std::list<std::string>{
          "shaders/border_junction.vert",
          "shaders/flat_skeleton.frag"
        });
        m_skeletons = new median_path::median_skeletons_renderable( skeleton_program, isolated_program, border_junction_program );
        add_renderable( m_skeletons );
      }
    else
      {
        for( int i = 0; i < number_of_geometries; ++ i )
          {
            for( int j = 0; j < number_of_topologies; ++ j )
              {
                m_skeletons->remove( m_handles[i][j] );
                m_handles[i][j] = median_path::median_skeletons_renderable::handle();
              }
          }
      }

    if( geometries.empty() )
      {
        m_handles[geometry_method::VORONOI_BALLS][topology_method::VORONOI] = m_skeletons->add( benchmark_directory + "/voronoi/voronoi/" + shape_stem + "." + extension );
        m_handles[geometry_method::VORONOI_BALLS][topology_method::POWERSHAPE] = m_skeletons->add( benchmark_directory + "/voronoi/powershape/" + shape_stem + "." + extension );
        m_handles[geometry_method::VORONOI_BALLS][topology_method::DELAUNAY_RECONSTRUCTION] = m_skeletons->add( benchmark_directory + "/voronoi/delaunay/" + shape_stem + "." + extension );
        m_handles[geometry_method::VORONOI_BALLS][topology_method::WEIGHTED_ALPHA_SHAPE] = m_skeletons->add( benchmark_directory + "/voronoi/weighted_alpha_shape/" + shape_stem + "." + extension );

        m_handles[geometry_method::POLAR_BALLS][topology_method::POWERSHAPE] = m_skeletons->add( benchmark_directory + "/powershape/powershape/" + shape_stem + "." + extension );
        m_handles[geometry_method::POLAR_BALLS][topology_method::DELAUNAY_RECONSTRUCTION] = m_skeletons->add( benchmark_directory + "/powershape/delaunay/" + shape_stem + "." + extension );
        m_handles[geometry_method::POLAR_BALLS][topology_method::WEIGHTED_ALPHA_SHAPE] = m_skeletons->add( benchmark_directory + "/powershape/weighted_alpha_shape/" + shape_stem + "." + extension );

        m_handles[geometry_method::SHRINKING_BALLS][topology_method::DELAUNAY_RECONSTRUCTION] = m_skeletons->add( benchmark_directory + "/shrinking_ball/delaunay/" + shape_stem + "." + extension );
        m_handles[geometry_method::SHRINKING_BALLS][topology_method::WEIGHTED_ALPHA_SHAPE] = m_skeletons->add( benchmark_directory + "/shrinking_ball/weighted_alpha_shape/" + shape_stem + "." + extension );
      }
    else
      {
        std::sort( geometries.begin(), geometries.end() );
        auto last = std::unique( geometries.begin(), geometries.end() );
        geometries.erase( last, geometries.end() );
        for( auto& g : geometries )
          {
            switch( g )
            {
              case geometry_method::VORONOI_BALLS:
                m_handles[geometry_method::VORONOI_BALLS][topology_method::VORONOI] = m_skeletons->add( benchmark_directory + "/voronoi/voronoi/" + shape_stem + "." + extension );
                m_handles[geometry_method::VORONOI_BALLS][topology_method::POWERSHAPE] = m_skeletons->add( benchmark_directory + "/voronoi/powershape/" + shape_stem + "." + extension );
                m_handles[geometry_method::VORONOI_BALLS][topology_method::DELAUNAY_RECONSTRUCTION] = m_skeletons->add( benchmark_directory + "/voronoi/delaunay/" + shape_stem + "." + extension );
                m_handles[geometry_method::VORONOI_BALLS][topology_method::WEIGHTED_ALPHA_SHAPE] = m_skeletons->add( benchmark_directory + "/voronoi/weighted_alpha_shape/" + shape_stem + "." + extension );
                break;

              case geometry_method::POLAR_BALLS:
                m_handles[geometry_method::POLAR_BALLS][topology_method::POWERSHAPE] = m_skeletons->add( benchmark_directory + "/powershape/powershape/" + shape_stem + "." + extension );
                m_handles[geometry_method::POLAR_BALLS][topology_method::DELAUNAY_RECONSTRUCTION] = m_skeletons->add( benchmark_directory + "/powershape/delaunay/" + shape_stem + "." + extension );
                m_handles[geometry_method::POLAR_BALLS][topology_method::WEIGHTED_ALPHA_SHAPE] = m_skeletons->add( benchmark_directory + "/powershape/weighted_alpha_shape/" + shape_stem + "." + extension );
                break;

              case geometry_method::SHRINKING_BALLS:
                m_handles[geometry_method::SHRINKING_BALLS][topology_method::DELAUNAY_RECONSTRUCTION] = m_skeletons->add( benchmark_directory + "/shrinking_ball/delaunay/" + shape_stem + "." + extension );
                m_handles[geometry_method::SHRINKING_BALLS][topology_method::WEIGHTED_ALPHA_SHAPE] = m_skeletons->add( benchmark_directory + "/shrinking_ball/weighted_alpha_shape/" + shape_stem + "." + extension );
                break;
            }
          }
      }

    bool found_active = false;
    for( int i = 0; i < number_of_geometries; ++ i )
      {
        for( int j = 0; j < number_of_topologies; ++ j )
          {
            if( m_handles[i][j].is_valid() )
              {
                m_skeletons->get( m_handles[i][j] ).active = true;
                m_geometry = static_cast<geometry_method>(i);
                m_topology = static_cast<topology_method>(j);
                found_active = true;
                break;
              }
          }
        if( found_active )
          break;
      }

    m_skeletons->render_triangles( true );
    m_skeletons->render_isolated_atoms( false );
    m_skeletons->render_isolated_links( false );
    m_skeletons->render_borders_junctions( false );
    m_skeletons->render_wireframe( true );
    m_skeletons->use_radii_colors( true );

    emit_available_geometry_methods_has_changed();
    emit_available_topology_methods_has_changed();
    emit_active_geometry_method_has_changed();
    emit_active_topology_method_has_changed();
  }

  bool simple_gl_window::request_voronoi_geometry()
  {
    if( m_geometry != geometry_method::VORONOI_BALLS)
      {
        if( m_handles[ m_geometry ][ m_topology ].is_valid() )
          m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = false;
        m_geometry = geometry_method::VORONOI_BALLS;
        emit_active_geometry_method_has_changed();
        emit_available_topology_methods_has_changed();
        if( m_handles[ m_geometry ][ m_topology ].is_valid() )
          {
            m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = true;
            return true;
          }
        return false;
      }
    return true;
  }
  bool simple_gl_window::request_polar_geometry()
  {
    if( m_geometry != geometry_method::POLAR_BALLS )
      {
        if( m_handles[ m_geometry ][ m_topology ].is_valid() )
          m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = false;
        m_geometry = geometry_method::POLAR_BALLS;
        emit_active_geometry_method_has_changed();
        emit_available_topology_methods_has_changed();
        if( m_handles[ m_geometry ][ m_topology ].is_valid() )
          {
            m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = true;
            return true;
          }
        return false;
      }
    return true;
  }
  bool simple_gl_window::request_shrinking_geometry()
  {
    if( m_geometry != geometry_method::SHRINKING_BALLS )
      {
        if( m_handles[ m_geometry ][ m_topology ].is_valid() )
          m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = false;
        m_geometry = geometry_method::SHRINKING_BALLS;
        emit_active_geometry_method_has_changed();
        emit_available_topology_methods_has_changed();
        if( m_handles[ m_geometry ][ m_topology ].is_valid() )
          {
            m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = true;
            return true;
          }
        return false;
      }
    return true;
  }

  bool simple_gl_window::request_voronoi_reconstruction()
  {
    if( m_handles[ m_geometry ][ m_topology ].is_valid() )
      m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = false;
    m_topology = topology_method::VORONOI;
    emit_active_topology_method_has_changed();
    if( m_handles[ m_geometry ][ m_topology ].is_valid() )
      {
        m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = true;
        return true;
      }
    return false;
  }
  bool simple_gl_window::request_powershape_reconstruction()
  {
    if( m_handles[ m_geometry ][ m_topology ].is_valid() )
      m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = false;
    m_topology = topology_method::POWERSHAPE;
    emit_active_topology_method_has_changed();
    if( m_handles[ m_geometry ][ m_topology ].is_valid() )
      {
        m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = true;
        return true;
      }
    return false;
  }
  bool simple_gl_window::request_delaunay_reconstruction()
  {
    if( m_handles[ m_geometry ][ m_topology ].is_valid() )
      m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = false;
    m_topology = topology_method::DELAUNAY_RECONSTRUCTION;
    emit_active_topology_method_has_changed();
    if( m_handles[ m_geometry ][ m_topology ].is_valid() )
      {
        m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = true;
        return true;
      }
    return false;
  }
  bool simple_gl_window::request_weighted_alpha_reconstruction()
  {
    if( m_handles[ m_geometry ][ m_topology ].is_valid() )
      m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = false;
    m_topology = topology_method::WEIGHTED_ALPHA_SHAPE;
    emit_active_topology_method_has_changed();
    if( m_handles[ m_geometry ][ m_topology ].is_valid() )
      {
        m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = true;
        return true;
      }
    return false;
  }

  void simple_gl_window::render_border_junction_links( bool render )
  {
    if( m_skeletons )
      m_skeletons->render_borders_junctions( render );
  }

  void simple_gl_window::render_isolated_atoms( bool render )
  {
    if( m_skeletons )
      m_skeletons->render_isolated_atoms( render );
  }
  void simple_gl_window::render_isolated_links( bool render )
  {
    if( m_skeletons )
      m_skeletons->render_isolated_links( render );
  }
  void simple_gl_window::render_triangles( bool render )
  {
    if( m_skeletons )
      m_skeletons->render_triangles( render );
  }
  void simple_gl_window::render_wireframe( bool render )
  {
    if( m_skeletons )
      m_skeletons->render_wireframe( render );
  }
  void simple_gl_window::use_radii_colors( bool use )
  {
    if( m_skeletons )
      m_skeletons->use_radii_colors( use );
  }

  void simple_gl_window::set_atom_color( const QColor& color )
  {
    if( m_skeletons )
      m_skeletons->set_atom_color( median_path::gpu_vec4{ color.redF(), color.greenF(), color.blueF(), 1.0 } );
  }


  bool simple_gl_window::get_has_voronoi_geometry() const
  {
    for( int i = 0; i < number_of_topologies; ++ i )
      {
        if( m_handles[ geometry_method::VORONOI_BALLS ][ i ].is_valid() )
          return true;
      }
    return false;
  }
  bool simple_gl_window::get_has_polar_geometry() const
  {
    for( int i = 0; i < number_of_topologies; ++ i )
      {
        if( m_handles[ geometry_method::POLAR_BALLS ][ i ].is_valid() )
          return true;
      }
    return false;
  }
  bool simple_gl_window::get_has_shrinking_geometry() const
  {
    for( int i = 0; i < number_of_topologies; ++ i )
      {
        if( m_handles[ geometry_method::SHRINKING_BALLS ][ i ].is_valid() )
          return true;
      }
    return false;
  }

  bool simple_gl_window::get_voronoi_geometry_active() const
  {
    return m_geometry == geometry_method::VORONOI_BALLS;
  }
  bool simple_gl_window::get_polar_geometry_active() const
  {
    return m_geometry == geometry_method::POLAR_BALLS;
  }
  bool simple_gl_window::get_shrinking_geometry_active() const
  {
    return m_geometry == geometry_method::SHRINKING_BALLS;
  }

  bool simple_gl_window::get_has_voronoi_reconstruction() const
  {
    return m_handles[ m_geometry ][ topology_method::VORONOI ].is_valid();
  }
  bool simple_gl_window::get_has_powershape_reconstruction() const
  {
    return m_handles[ m_geometry ][ topology_method::POWERSHAPE ].is_valid();
  }
  bool simple_gl_window::get_has_delaunay_reconstruction() const
  {
    return m_handles[ m_geometry ][ topology_method::DELAUNAY_RECONSTRUCTION ].is_valid();
  }
  bool simple_gl_window::get_has_weighted_alpha_reconstruction() const
  {
    return m_handles[ m_geometry ][ topology_method::WEIGHTED_ALPHA_SHAPE ].is_valid();
  }

  bool simple_gl_window::get_voronoi_reconstruction_active() const
  {
    return m_topology == topology_method::VORONOI;
  }
  bool simple_gl_window::get_powershape_reconstruction_active() const
  {
    return m_topology == topology_method::POWERSHAPE;
  }
  bool simple_gl_window::get_delaunay_reconstruction_active() const
  {
    return m_topology == topology_method::DELAUNAY_RECONSTRUCTION;
  }
  bool simple_gl_window::get_weighted_alpha_reconstruction_active() const
  {
    return m_topology == topology_method::WEIGHTED_ALPHA_SHAPE;
  }
  void simple_gl_window::emit_active_geometry_method_has_changed()
  {
    emit voronoi_geometry_active_changed();
    emit polar_geometry_active_changed();
    emit shrinking_geometry_active_changed();
  }
  void simple_gl_window::emit_active_topology_method_has_changed()
  {
    emit voronoi_reconstruction_active_changed();
    emit delaunay_reconstruction_active_changed();
    emit powershape_reconstruction_active_changed();
    emit weighted_alpha_reconstruction_active_changed();
  }
  void simple_gl_window::emit_available_geometry_methods_has_changed()
  {
    emit has_voronoi_geometry_changed();
    emit has_polar_geometry_changed();
    emit has_shrinking_geometry_changed();
  }
  void simple_gl_window::emit_available_topology_methods_has_changed()
  {
    emit has_voronoi_reconstruction_changed();
    emit has_delaunay_reconstruction_changed();
    emit has_powershape_reconstruction_changed();
    emit has_weighted_alpha_reconstruction_changed();
  }
