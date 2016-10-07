/*  Created on: Mar 31, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "benchmark_viewer/benchmark_viewer_window.h"
# include "simple_gl_renderer.h"

# include <graphics-origin/application/shader_program.h>
# include <graphics-origin/tools/log.h>
# include <graphics-origin/application/camera.h>

# include <fstream>

  constexpr int benchmark_viewer_window::number_of_geometries;
  constexpr int benchmark_viewer_window::number_of_topologies;


  benchmark_viewer_window::benchmark_viewer_window( QQuickItem* parent )
    : graphics_origin::application::window( parent ), m_skeletons{ nullptr }, m_geometry{geometry_method::VORONOI_BALLS}, m_topology{topology_method::DELAUNAY_RECONSTRUCTION}
  {
    initialize_renderer( new median_path::simple_gl_renderer );
  }

  void benchmark_viewer_window::save_camera(const QString& filename )
  {
    std::string f = filename.toUtf8().constData();
    if( !f.empty() )
      {
        std::ofstream output( f );
        if( output.is_open() )
          {
            output << threaded_renderer->get_view_matrix();
            output.close();
            LOG( info, "camera view matrix saved to file [" << f << "]");
          }
        else
          {
            LOG( error, "cannot save to file [" << f << "]");
          }
      }
  }

  void benchmark_viewer_window::load_camera( const QString& filename )
  {
    std::string f = filename.toUtf8().constData();
    if( !f.empty() )
      {
        std::ifstream input( f );
        if( input.is_open() )
          {
            median_path::gl_mat4 view;
            input >> view;
            if( !input.fail() )
              {
                dynamic_cast< graphics_origin::application::camera* >(get_camera())->set_view_matrix( view );
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

  void benchmark_viewer_window::reset_camera()
  {
    dynamic_cast< graphics_origin::application::camera* >(get_camera())->set_view_matrix( glm::lookAt( median_path::gl_vec3{3,0,0}, median_path::gl_vec3{}, median_path::gl_vec3{0,0,1}) );
  }


  void
  benchmark_viewer_window::load_benchmark(
      const std::string& shape_stem, const std::string& benchmark_directory, const std::string& extension,
      std::vector< median_path::skeletonizer::parameters::geometry_method>& geometries,
      std::vector< median_path::structurer::parameters::topology_method>& topologies )
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
        auto balls_program = std::make_shared<graphics_origin::application::shader_program>( std::list<std::string>{
          "shaders/balls.vert",
          "shaders/balls.frag",
          "shaders/balls.geom"
        });
        m_skeletons = new median_path::median_skeletons_renderable( skeleton_program, isolated_program, border_junction_program, balls_program );
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
        geometries.push_back( geometry_method::VORONOI_BALLS );
        geometries.push_back( geometry_method::POLAR_BALLS );
        geometries.push_back( geometry_method::SHRINKING_BALLS );
      }
    else
      {
        std::sort( geometries.begin(), geometries.end() );
        geometries.erase( std::unique( geometries.begin(), geometries.end() ), geometries.end() );
      }
    if( topologies.empty() )
      {
        topologies.push_back( topology_method::VORONOI );   
        topologies.push_back( topology_method::POWERSHAPE );
        topologies.push_back( topology_method::DELAUNAY_RECONSTRUCTION );
        topologies.push_back( topology_method::WEIGHTED_ALPHA_SHAPE );
      }
    else
      {
        std::sort( topologies.begin(), topologies.end() );
        topologies.erase( std::unique( topologies.begin(), topologies.end() ), topologies.end() );
      }
      
      
    for( auto& g : geometries )
        {
        switch( g )
        {
            case geometry_method::VORONOI_BALLS:
                
                if( std::find( topologies.begin(), topologies.end(), topology_method::VORONOI ) != topologies.end() )
            m_handles[geometry_method::VORONOI_BALLS][topology_method::VORONOI] = m_skeletons->add( benchmark_directory + "/voronoi/voronoi/" + shape_stem + "." + extension );
                if( std::find( topologies.begin(), topologies.end(), topology_method::POWERSHAPE ) != topologies.end() )
            m_handles[geometry_method::VORONOI_BALLS][topology_method::POWERSHAPE] = m_skeletons->add( benchmark_directory + "/voronoi/powershape/" + shape_stem + "." + extension );
            if( std::find( topologies.begin(), topologies.end(), topology_method::DELAUNAY_RECONSTRUCTION ) != topologies.end() )
            m_handles[geometry_method::VORONOI_BALLS][topology_method::DELAUNAY_RECONSTRUCTION] = m_skeletons->add( benchmark_directory + "/voronoi/delaunay/" + shape_stem + "." + extension );
            if( std::find( topologies.begin(), topologies.end(), topology_method::WEIGHTED_ALPHA_SHAPE ) != topologies.end() )
            m_handles[geometry_method::VORONOI_BALLS][topology_method::WEIGHTED_ALPHA_SHAPE] = m_skeletons->add( benchmark_directory + "/voronoi/weighted_alpha_shape/" + shape_stem + "." + extension );
            break;

            case geometry_method::POLAR_BALLS:
                if( std::find( topologies.begin(), topologies.end(), topology_method::POWERSHAPE ) != topologies.end() )
            m_handles[geometry_method::POLAR_BALLS][topology_method::POWERSHAPE] = m_skeletons->add( benchmark_directory + "/powershape/powershape/" + shape_stem + "." + extension );
            if( std::find( topologies.begin(), topologies.end(), topology_method::DELAUNAY_RECONSTRUCTION ) != topologies.end() )
            m_handles[geometry_method::POLAR_BALLS][topology_method::DELAUNAY_RECONSTRUCTION] = m_skeletons->add( benchmark_directory + "/powershape/delaunay/" + shape_stem + "." + extension );
            if( std::find( topologies.begin(), topologies.end(), topology_method::WEIGHTED_ALPHA_SHAPE ) != topologies.end() )
            m_handles[geometry_method::POLAR_BALLS][topology_method::WEIGHTED_ALPHA_SHAPE] = m_skeletons->add( benchmark_directory + "/powershape/weighted_alpha_shape/" + shape_stem + "." + extension );
            break;

            case geometry_method::SHRINKING_BALLS:
            if( std::find( topologies.begin(), topologies.end(), topology_method::DELAUNAY_RECONSTRUCTION ) != topologies.end() )
            m_handles[geometry_method::SHRINKING_BALLS][topology_method::DELAUNAY_RECONSTRUCTION] = m_skeletons->add( benchmark_directory + "/shrinking_ball/delaunay/" + shape_stem + "." + extension );
            if( std::find( topologies.begin(), topologies.end(), topology_method::WEIGHTED_ALPHA_SHAPE ) != topologies.end() )
            m_handles[geometry_method::SHRINKING_BALLS][topology_method::WEIGHTED_ALPHA_SHAPE] = m_skeletons->add( benchmark_directory + "/shrinking_ball/weighted_alpha_shape/" + shape_stem + "." + extension );
            break;
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

  bool benchmark_viewer_window::request_voronoi_geometry()
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
  bool benchmark_viewer_window::request_polar_geometry()
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
  bool benchmark_viewer_window::request_shrinking_geometry()
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

  bool benchmark_viewer_window::request_voronoi_reconstruction()
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
  bool benchmark_viewer_window::request_powershape_reconstruction()
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
  bool benchmark_viewer_window::request_delaunay_reconstruction()
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
  bool benchmark_viewer_window::request_weighted_alpha_reconstruction()
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

  void benchmark_viewer_window::render_border_junction_links( bool render )
  {
    if( m_skeletons )
      m_skeletons->render_borders_junctions( render );
  }

  void benchmark_viewer_window::render_isolated_atoms( bool render )
  {
    if( m_skeletons )
      m_skeletons->render_isolated_atoms( render );
  }
  void benchmark_viewer_window::render_isolated_links( bool render )
  {
    if( m_skeletons )
      m_skeletons->render_isolated_links( render );
  }
  void benchmark_viewer_window::render_skeleton_points( bool render )
  {
    if( m_skeletons )
      m_skeletons->render_skeleton_points( render );
  }
  void benchmark_viewer_window::render_balls( bool render )
  {
    if( m_skeletons )
      m_skeletons->render_balls( render );
  }
  void benchmark_viewer_window::render_triangles( bool render )
  {
    if( m_skeletons )
      m_skeletons->render_triangles( render );
  }
  void benchmark_viewer_window::render_wireframe( bool render )
  {
    if( m_skeletons )
      m_skeletons->render_wireframe( render );
  }
  void benchmark_viewer_window::use_radii_colors( bool use )
  {
    if( m_skeletons )
      m_skeletons->use_radii_colors( use );
  }

  void benchmark_viewer_window::set_atom_color( const QColor& color )
  {
    if( m_skeletons )
      m_skeletons->set_atom_color( median_path::gl_vec4{ color.redF(), color.greenF(), color.blueF(), 1.0 } );
  }

  void benchmark_viewer_window::set_isolated_color( const QColor& color )
  {
    if( m_skeletons )
      m_skeletons->set_isolated_color( median_path::gl_vec4{ color.redF(), color.greenF(), color.blueF(), 1.0 } );
  }

  bool benchmark_viewer_window::get_has_voronoi_geometry() const
  {
    for( int i = 0; i < number_of_topologies; ++ i )
      {
        if( m_handles[ geometry_method::VORONOI_BALLS ][ i ].is_valid() )
          return true;
      }
    return false;
  }
  bool benchmark_viewer_window::get_has_polar_geometry() const
  {
    for( int i = 0; i < number_of_topologies; ++ i )
      {
        if( m_handles[ geometry_method::POLAR_BALLS ][ i ].is_valid() )
          return true;
      }
    return false;
  }
  bool benchmark_viewer_window::get_has_shrinking_geometry() const
  {
    for( int i = 0; i < number_of_topologies; ++ i )
      {
        if( m_handles[ geometry_method::SHRINKING_BALLS ][ i ].is_valid() )
          return true;
      }
    return false;
  }

  bool benchmark_viewer_window::get_voronoi_geometry_active() const
  {
    return m_geometry == geometry_method::VORONOI_BALLS;
  }
  bool benchmark_viewer_window::get_polar_geometry_active() const
  {
    return m_geometry == geometry_method::POLAR_BALLS;
  }
  bool benchmark_viewer_window::get_shrinking_geometry_active() const
  {
    return m_geometry == geometry_method::SHRINKING_BALLS;
  }

  bool benchmark_viewer_window::get_has_voronoi_reconstruction() const
  {
    return m_handles[ m_geometry ][ topology_method::VORONOI ].is_valid();
  }
  bool benchmark_viewer_window::get_has_powershape_reconstruction() const
  {
    return m_handles[ m_geometry ][ topology_method::POWERSHAPE ].is_valid();
  }
  bool benchmark_viewer_window::get_has_delaunay_reconstruction() const
  {
    return m_handles[ m_geometry ][ topology_method::DELAUNAY_RECONSTRUCTION ].is_valid();
  }
  bool benchmark_viewer_window::get_has_weighted_alpha_reconstruction() const
  {
    return m_handles[ m_geometry ][ topology_method::WEIGHTED_ALPHA_SHAPE ].is_valid();
  }

  bool benchmark_viewer_window::get_voronoi_reconstruction_active() const
  {
    return m_topology == topology_method::VORONOI;
  }
  bool benchmark_viewer_window::get_powershape_reconstruction_active() const
  {
    return m_topology == topology_method::POWERSHAPE;
  }
  bool benchmark_viewer_window::get_delaunay_reconstruction_active() const
  {
    return m_topology == topology_method::DELAUNAY_RECONSTRUCTION;
  }
  bool benchmark_viewer_window::get_weighted_alpha_reconstruction_active() const
  {
    return m_topology == topology_method::WEIGHTED_ALPHA_SHAPE;
  }
  void benchmark_viewer_window::emit_active_geometry_method_has_changed()
  {
    emit voronoi_geometry_active_changed();
    emit polar_geometry_active_changed();
    emit shrinking_geometry_active_changed();
  }
  void benchmark_viewer_window::emit_active_topology_method_has_changed()
  {
    emit voronoi_reconstruction_active_changed();
    emit delaunay_reconstruction_active_changed();
    emit powershape_reconstruction_active_changed();
    emit weighted_alpha_reconstruction_active_changed();
  }
  void benchmark_viewer_window::emit_available_geometry_methods_has_changed()
  {
    emit has_voronoi_geometry_changed();
    emit has_polar_geometry_changed();
    emit has_shrinking_geometry_changed();
  }
  void benchmark_viewer_window::emit_available_topology_methods_has_changed()
  {
    emit has_voronoi_reconstruction_changed();
    emit has_delaunay_reconstruction_changed();
    emit has_powershape_reconstruction_changed();
    emit has_weighted_alpha_reconstruction_changed();
  }
