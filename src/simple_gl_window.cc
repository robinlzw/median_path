/*  Created on: Mar 31, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "simple_gl_window.h"
# include "simple_gl_renderer.h"


# include <graphics-origin/application/shader_program.h>
# include <graphics-origin/tools/log.h>


  simple_gl_window::simple_gl_window( QQuickItem* parent )
    : graphics_origin::application::gl_window( parent ), m_skeletons{ nullptr }, m_geometry{0}, m_topology{0}
  {
    initialize_renderer( new median_path::simple_gl_renderer );
  }

  void
  simple_gl_window::load_benchmark( const std::string& shape_stem, const std::string& benchmark_directory, const std::string& extension )
  {
    auto skeleton_program = std::make_shared<graphics_origin::application::shader_program>( std::list<std::string>{
      "shaders/skeleton.vert",
      "shaders/skeleton.geom",
      "shaders/skeleton.frag"
    });


    m_skeletons = new median_path::median_skeletons_renderable( skeleton_program );

    m_handles[voronoi_geometry][voronoi_reconstruction] = m_skeletons->add( benchmark_directory + "/voronoi/voronoi/" + shape_stem + "." + extension );
    m_handles[voronoi_geometry][powershape_reconstruction] = m_skeletons->add( benchmark_directory + "/voronoi/powershape/" + shape_stem + "." + extension );
    m_handles[voronoi_geometry][delaunay_reconstruction] = m_skeletons->add( benchmark_directory + "/voronoi/delaunay/" + shape_stem + "." + extension );
    m_handles[voronoi_geometry][weighted_alpha_reconstruction] = m_skeletons->add( benchmark_directory + "/voronoi/weighted_alpha_shape/" + shape_stem + "." + extension );

    m_handles[polar_geometry][powershape_reconstruction] = m_skeletons->add( benchmark_directory + "/powershape/powershape/" + shape_stem + "." + extension );
    m_handles[polar_geometry][delaunay_reconstruction] = m_skeletons->add( benchmark_directory + "/powershape/delaunay/" + shape_stem + "." + extension );
    m_handles[polar_geometry][weighted_alpha_reconstruction] = m_skeletons->add( benchmark_directory + "/powershape/weighted_alpha_shape/" + shape_stem + "." + extension );

    m_handles[shrinking_geometry][delaunay_reconstruction] = m_skeletons->add( benchmark_directory + "/shrinking_ball/delaunay/" + shape_stem + "." + extension );
    m_handles[shrinking_geometry][weighted_alpha_reconstruction] = m_skeletons->add( benchmark_directory + "/shrinking_ball/weighted_alpha_shape/" + shape_stem + "." + extension );

    m_skeletons->get( m_handles[voronoi_geometry][delaunay_reconstruction] ).active = true;

    add_renderable( m_skeletons );
  }

  void simple_gl_window::request_voronoi_geometry()
  {
    if( m_geometry != voronoi_geometry )
      {
        if( m_handles[ m_geometry ][ m_topology ].is_valid() )
          m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = false;
        m_geometry = voronoi_geometry;
        if( m_handles[ m_geometry ][ m_topology ].is_valid() )
          m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = true;
      }
  }
  void simple_gl_window::request_polar_geometry()
  {
    if( m_geometry != polar_geometry )
      {
        if( m_handles[ m_geometry ][ m_topology ].is_valid() )
          m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = false;
        m_geometry = polar_geometry;
        if( m_handles[ m_geometry ][ m_topology ].is_valid() )
          m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = true;
      }
  }
  void simple_gl_window::request_shrinking_geometry()
  {
    if( m_geometry != shrinking_geometry )
      {
        if( m_handles[ m_geometry ][ m_topology ].is_valid() )
          m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = false;
        m_geometry = shrinking_geometry;
        if( m_handles[ m_geometry ][ m_topology ].is_valid() )
          m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = true;
      }
  }

  void simple_gl_window::request_voronoi_reconstruction()
  {
    if( m_topology != voronoi_reconstruction )
      {
        if( m_handles[ m_geometry ][ m_topology ].is_valid() )
          m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = false;
        m_topology = voronoi_reconstruction;
        if( m_handles[ m_geometry ][ m_topology ].is_valid() )
          m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = true;
      }
  }
  void simple_gl_window::request_powershape_reconstruction()
  {
    if( m_topology != powershape_reconstruction )
      {
        if( m_handles[ m_geometry ][ m_topology ].is_valid() )
          m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = false;
        m_topology = powershape_reconstruction;
        if( m_handles[ m_geometry ][ m_topology ].is_valid() )
          m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = true;
      }
  }
  void simple_gl_window::request_delaunay_reconstruction()
  {
    if( m_topology != delaunay_reconstruction )
      {
        if( m_handles[ m_geometry ][ m_topology ].is_valid() )
          m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = false;
        m_topology = delaunay_reconstruction;
        if( m_handles[ m_geometry ][ m_topology ].is_valid() )
          m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = true;
      }
  }
  void simple_gl_window::request_weighted_alpha_reconstruction()
  {
    if( m_topology != weighted_alpha_reconstruction )
      {
        if( m_handles[ m_geometry ][ m_topology ].is_valid() )
          m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = false;
        m_topology = weighted_alpha_reconstruction;
        if( m_handles[ m_geometry ][ m_topology ].is_valid() )
          m_skeletons->get( m_handles[ m_geometry ][ m_topology ] ).active = true;
      }
  }
