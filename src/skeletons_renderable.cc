/*  Created on: Mar 29, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "skeletons_renderable.h"

# include <graphics-origin/application/gl_window_renderer.h>
# include <graphics-origin/application/gl_helper.h>
# include <graphics-origin/application/camera.h>
# include <graphics-origin/geometry/vec.h>

# include <GL/glew.h>
BEGIN_MP_NAMESPACE
  median_skeletons_renderable::storage&
  median_skeletons_renderable::storage::operator=( storage&& other )
  {
    skeleton = std::move( other.skeleton );
    for( int id = 0; id < number_of_buffers; ++ id )
      buffer_ids[id] = other.buffer_ids[id];
    number_of_atoms = other.number_of_atoms;
    number_of_isolated_atoms = other.number_of_isolated_atoms;
    number_of_isolated_links = other.number_of_isolated_links;
    number_of_border_links = other.number_of_border_links;
    number_of_junction_links = other.number_of_junction_links;
    vao = other.vao;
    dirty = other.dirty;
    active = other.active;
    destroyed = other.destroyed;
    return *this;
  }

  median_skeletons_renderable::storage::storage()
    : vao{ 0 },
      number_of_atoms{0}, number_of_isolated_atoms{0}, number_of_isolated_links{0},
      number_of_border_links{0}, number_of_junction_links{0},
      dirty{true}, active{ false }, destroyed{false}
  {
    for( int id = 0; id < number_of_buffers; ++ id )
      buffer_ids[id] = 0;
  }


  median_skeletons_renderable::median_skeletons_renderable(
      graphics_origin::application::shader_program_ptr program,
      graphics_origin::application::shader_program_ptr isolated,
      graphics_origin::application::shader_program_ptr border_junction,
      graphics_origin::application::shader_program_ptr ball )
    : m_atom_color{ 0.56, 0.619, 0.764, 1.0 }, m_isolated_color{0,0,0,1.0},
      m_points_and_line_program{ isolated }, m_border_junction_program{ border_junction }, m_ball_program{ ball },
      m_render_balls{false}, m_render_skeleton_points{false}, m_render_triangles{true},
      m_render_isolated_atoms{ false }, m_render_isolated_links{ false }, m_render_borders_junctions{ false },
      m_render_wireframe{ false }, m_use_radii_colors{ true }
  {
    model = gl_mat4(1.0);
    program = program;
  }

  median_skeletons_renderable::~median_skeletons_renderable()
  {
    remove_gpu_data();
  }

  void
  median_skeletons_renderable::remove_gpu_data()
  {
    auto size = m_skeletons.get_size();
    auto data = m_skeletons.data();
    for( decltype(size) i = 0; i < size; ++ i, ++data )
      {
        glcheck(glDeleteVertexArrays( 1, &data->vao ));
        glcheck(glDeleteBuffers( number_of_buffers, data->buffer_ids));
        data->vao = 0;
      }
  }

  void
  median_skeletons_renderable::update_gpu_data()
  {
    std::vector< median_skeleton::atom_index > indices;
    std::vector<gl_vec4> colors;
    std::vector< median_skeleton::atom_index > isolated;
    std::vector< median_skeleton::atom_index > borders;
    std::vector< median_skeleton::atom_index > junctions;
    auto data = m_skeletons.data();
    for( size_t i = 0; i < m_skeletons.get_size();  )
      {
        if( data->destroyed )
          {
            data->destroyed = false;
            glcheck(glDeleteVertexArrays( 1, &data->vao));
            glcheck(glDeleteBuffers( number_of_buffers, data->buffer_ids ));
            m_skeletons.remove( data );
          }
        else
          {

          if( data->dirty && data->skeleton.get_number_of_atoms() )
            {
              if( !data->vao )
                {
                  glcheck(glGenVertexArrays( 1, &data->vao));
                  glcheck(glGenBuffers( number_of_buffers, data->buffer_ids));
                }

                const median_skeleton::atom_index nbatoms = data->skeleton.get_number_of_atoms();
                data->number_of_atoms = nbatoms;
                int  atom_location = program->get_attribute_location( "atom" );
                int color_location = program->get_attribute_location( "color");


                glcheck(glBindVertexArray( data->vao ));
                  glcheck(glBindBuffer( GL_ARRAY_BUFFER, data->buffer_ids[balls_vbo]));
                  glcheck(glBufferData( GL_ARRAY_BUFFER, sizeof(median_skeleton::atom) * nbatoms,
                    &data->skeleton.get_atom_by_index( 0 ), GL_STATIC_DRAW ));
                  glcheck(glEnableVertexAttribArray( atom_location ));
                  glcheck(glVertexAttribPointer( atom_location,
                    4, GL_DOUBLE, GL_FALSE,
                    0, 0 ));


                  real minr, maxr;
                  data->skeleton.compute_minmax_radii( minr, maxr );
                  isolated.resize( 0 );
                  colors.resize( nbatoms );
                  # pragma omp parallel for
                  for( median_skeleton::atom_index j = 0; j < nbatoms; ++ j )
                    {
                      colors[ j ] = gl_vec4(graphics_origin::get_color( data->skeleton.get_atom_by_index( j ).w, minr, maxr ), 1.0 );

                      if( !data->skeleton.get_number_of_links( j ) )
                        {
                          # pragma omp critical
                          isolated.push_back( j );
                        }
                    }
                  glcheck(glBindBuffer( GL_ARRAY_BUFFER, data->buffer_ids[colors_vbo]));
                  glcheck(glBufferData( GL_ARRAY_BUFFER, sizeof(gl_vec4) * nbatoms, colors.data(), GL_STATIC_DRAW));
                  glcheck(glEnableVertexAttribArray( color_location ));
                  glcheck(glVertexAttribPointer( color_location,
                    4, GL_FLOAT, GL_FALSE,
                    0, 0 ));

                  data->number_of_isolated_atoms = isolated.size();
                  glcheck(glBindBuffer( GL_ARRAY_BUFFER, data->buffer_ids[isolated_vertices_ibo]));
                  glcheck(glBufferData( GL_ARRAY_BUFFER, data->number_of_isolated_atoms * sizeof(median_skeleton::atom_index), isolated.data(), GL_STATIC_DRAW));


                  const median_skeleton::link_index nblinks = data->skeleton.get_number_of_links();
                  indices.resize( nblinks * 2 );
                  isolated.resize( 0 );
                  borders.resize( 0 );
                  junctions.resize( 0 );
                  # pragma omp parallel for
                  for( median_skeleton::link_index j = 0; j < nblinks; ++ j )
                    {
                      const auto& link = data->skeleton.get_link_by_index( j );
                      size_t offset = j * 2;
                      indices[ offset    ] = data->skeleton.get_index( link.h1 );
                      indices[ offset + 1] = data->skeleton.get_index( link.h2 );
                      auto nbfaces = data->skeleton.get_number_of_faces( j );
                      if( !nbfaces )
                        {
                          #pragma omp critical
                          {
                            isolated.push_back( indices[ offset ] );
                            isolated.push_back( indices[ offset + 1 ] );
                          }
                        }
                      else if( nbfaces == 1 )
                        {
                          #pragma omp critical
                          {
                            borders.push_back( indices[ offset ] );
                            borders.push_back( indices[ offset + 1 ] );
                          }
                        }
                      else if( nbfaces > 2 )
                        {
                          #pragma omp critical
                          {
                            junctions.push_back( indices[ offset ] );
                            junctions.push_back( indices[ offset + 1 ] );
                          }
                        }
                    }

                  glcheck(glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, data->buffer_ids[links_ibo]));
                  glcheck(glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(median_skeleton::atom_index), indices.data(), GL_STATIC_DRAW));

                  data->number_of_isolated_links = isolated.size() >> 1;
                  glcheck(glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, data->buffer_ids[isolated_links_ibo]));
                  glcheck(glBufferData( GL_ELEMENT_ARRAY_BUFFER, isolated.size() * sizeof(median_skeleton::atom_index), isolated.data(), GL_STATIC_DRAW));

                  data->number_of_border_links = borders.size() >> 1;
                  glcheck(glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, data->buffer_ids[border_links_ibo]));
                  glcheck(glBufferData( GL_ELEMENT_ARRAY_BUFFER, borders.size() * sizeof(median_skeleton::atom_index), borders.data(), GL_STATIC_DRAW));

                  data->number_of_junction_links = junctions.size() >> 1;
                  glcheck(glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, data->buffer_ids[junction_links_ibo]));
                  glcheck(glBufferData( GL_ELEMENT_ARRAY_BUFFER, junctions.size() * sizeof(median_skeleton::atom_index), junctions.data(), GL_STATIC_DRAW));


                  const median_skeleton::face_index nbfaces = data->skeleton.get_number_of_faces();
                  indices.resize( nbfaces * 3 );
                  # pragma omp parallel for
                  for( median_skeleton::face_index j = 0; j < nbfaces; ++ j )
                    {
                      const auto& face = data->skeleton.get_face_by_index( j );
                      size_t offset = j * 3;
                      indices[ offset     ] = data->skeleton.get_index( face.atoms[ 0 ] );
                      indices[ offset + 1 ] = data->skeleton.get_index( face.atoms[ 1 ] );
                      indices[ offset + 2 ] = data->skeleton.get_index( face.atoms[ 2 ] );
                    }

                  glcheck(glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, data->buffer_ids[faces_ibo]));
                  glcheck(glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW));
                glcheck(glBindVertexArray(0));
                data->dirty = false;
              }
          }
        ++i;
        ++data;
      }
  }

  void
  median_skeletons_renderable::set_atom_color( const gl_vec4& color )
  {
    m_atom_color = color;
  }
  void
  median_skeletons_renderable::set_isolated_color( const gl_vec4& color )
  {
    m_isolated_color = color;
  }

  void
  median_skeletons_renderable::do_render()
  {
    glcheck(glUniform2fv( program->get_uniform_location( "window_dimensions"), 1, glm::value_ptr( m_renderer->get_window_dimensions())));
    glcheck(glUniformMatrix4fv( program->get_uniform_location( "model"), 1, GL_FALSE, glm::value_ptr( model )));
    glcheck(glUniformMatrix4fv( program->get_uniform_location( "view"), 1, GL_FALSE, glm::value_ptr( m_renderer->get_view_matrix() )));
    glcheck(glUniformMatrix4fv( program->get_uniform_location( "projection"), 1, GL_FALSE, glm::value_ptr( m_renderer->get_projection_matrix())));
    glcheck(glUniform1i( program->get_uniform_location( "grayscale" ), m_render_borders_junctions ));
    glcheck(glUniform1i( program->get_uniform_location( "wireframe" ), m_render_wireframe ));
    glcheck(glUniform1i( program->get_uniform_location( "use_atom_color"), m_use_radii_colors ));
    glcheck(glUniform4fv( program->get_uniform_location( "global_color"), 1, glm::value_ptr(m_atom_color)));

    auto size = m_skeletons.get_size();
    auto data = m_skeletons.data();
    if( m_render_triangles )
      {
        for( decltype(size) i = 0; i < size; ++ i, ++data )
          {
            if( data->active )
              {
                glcheck(glBindVertexArray( data->vao));
                glcheck(glDrawElements( GL_TRIANGLES, data->skeleton.get_number_of_faces() * 3, GL_UNSIGNED_INT, (void*)0));
              }
        }
        glcheck(glBindVertexArray( 0 ));
      }

    if( m_render_isolated_atoms || m_render_isolated_links || m_render_skeleton_points )
      {
        m_points_and_line_program->bind();
        glcheck(glUniform1i( m_points_and_line_program->get_uniform_location( "use_atom_color"), m_use_radii_colors ));
        glcheck(glUniform4fv( m_points_and_line_program->get_uniform_location( "global_color"), 1, glm::value_ptr(m_isolated_color)));
        glcheck(glUniformMatrix4fv( m_points_and_line_program->get_uniform_location( "mvp" ),
          1, GL_FALSE,
          glm::value_ptr( m_renderer->get_projection_matrix() * m_renderer->get_view_matrix() * model )));

        auto atom_location = m_points_and_line_program->get_attribute_location( "atom" );
        auto color_location = m_points_and_line_program->get_attribute_location( "color" );
        data = m_skeletons.data();
        for( decltype(size) i = 0; i < size; ++ i, ++ data )
          {
            if( data->active )
              {
                glcheck(glBindBuffer( GL_ARRAY_BUFFER, data->buffer_ids[balls_vbo]));
                glcheck(glEnableVertexAttribArray( atom_location ));
                glcheck(glVertexAttribPointer( atom_location, 4, GL_DOUBLE, GL_FALSE, 0, 0 ));

                glcheck(glBindBuffer( GL_ARRAY_BUFFER, data->buffer_ids[colors_vbo]));
                glcheck(glEnableVertexAttribArray( color_location ));
                glcheck(glVertexAttribPointer( color_location, 4, GL_FLOAT, GL_FALSE, 0, 0 ));

                if( m_render_skeleton_points )
                  {
                    glcheck(glPointSize( 4.0 ) );
                    glcheck(glDrawArrays( GL_POINTS, 0, data->number_of_atoms ));
                  }

                if( m_render_isolated_atoms )
                  {
                    glcheck(glPointSize( 6.0 ));
                    glcheck(glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, data->buffer_ids[isolated_vertices_ibo]));
                    glcheck(glDrawElements( GL_POINTS, data->number_of_isolated_atoms, GL_UNSIGNED_INT, (void*)0));
                  }

                if( m_render_isolated_links )
                  {
                    glcheck(glLineWidth( 4.0 ));
                    glcheck(glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, data->buffer_ids[isolated_links_ibo]));
                    glcheck(glDrawElements( GL_LINES, data->number_of_isolated_links << 1, GL_UNSIGNED_INT, (void*)0));
                  }
              }
          }
      }

    if( m_render_balls )
      {
        m_ball_program->bind();
        int ball_location  = m_ball_program->get_attribute_location(  "ball_attribute" );
        int color_location = m_ball_program->get_attribute_location( "color_attribute" );

        glcheck(glUniform1i( m_ball_program->get_uniform_location( "use_atom_color"), m_use_radii_colors ));
        glcheck(glUniform4fv( m_ball_program->get_uniform_location( "global_color"), 1, glm::value_ptr(m_atom_color)));
        glcheck(glUniformMatrix4fv( m_ball_program->get_uniform_location("projection"), 1, GL_FALSE, glm::value_ptr( m_renderer->get_projection_matrix() )));
        glcheck(glUniformMatrix4fv( m_ball_program->get_uniform_location( "mv"), 1, GL_FALSE, glm::value_ptr(m_renderer->get_view_matrix() * model)));

        data = m_skeletons.data();
        for( decltype(size) i = 0; i < size; ++ i, ++ data )
          {
            if( data->active )
              {
                glcheck(glBindBuffer( GL_ARRAY_BUFFER, data->buffer_ids[balls_vbo]));
                glcheck(glEnableVertexAttribArray( ball_location ));
                glcheck(glVertexAttribPointer( ball_location, 4, GL_DOUBLE, GL_FALSE, 0, 0 ));

                glcheck(glBindBuffer( GL_ARRAY_BUFFER, data->buffer_ids[colors_vbo]));
                glcheck(glEnableVertexAttribArray( color_location ));
                glcheck(glVertexAttribPointer( color_location, 4, GL_FLOAT, GL_FALSE, 0, 0 ));
                glcheck(glDrawArrays( GL_POINTS, 0, data->number_of_atoms ));
              }
          }
      }

    if( m_render_borders_junctions )
      {
        m_border_junction_program->bind();
        glcheck(glUniformMatrix4fv( m_border_junction_program->get_uniform_location( "mvp" ),
          1, GL_FALSE,
          glm::value_ptr( m_renderer->get_projection_matrix() * m_renderer->get_view_matrix() * model )));

        auto atom_location = m_border_junction_program->get_attribute_location( "atom" );
        glcheck(glLineWidth( 4.0 ));

        data = m_skeletons.data();
        for( decltype(size) i = 0; i < size; ++ i, ++ data )
          {
            if( data->active )
              {
                glcheck(glBindBuffer( GL_ARRAY_BUFFER, data->buffer_ids[balls_vbo]));
                glcheck(glEnableVertexAttribArray( atom_location ));
                glcheck(glVertexAttribPointer( atom_location, 4, GL_DOUBLE, GL_FALSE, 0, 0 ));

                glcheck(glUniform4fv( m_border_junction_program->get_uniform_location("color"), 1, glm::value_ptr(gl_vec4{1,0,0,1.0})));
                glcheck(glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, data->buffer_ids[border_links_ibo]));
                glcheck(glDrawElements( GL_LINES, data->number_of_border_links << 1, GL_UNSIGNED_INT, (void*)0));

                glcheck(glUniform4fv( m_border_junction_program->get_uniform_location("color"), 1, glm::value_ptr(gl_vec4{0,0,1,1.0})));
                glcheck(glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, data->buffer_ids[junction_links_ibo]));
                glcheck(glDrawElements( GL_LINES, data->number_of_junction_links << 1, GL_UNSIGNED_INT, (void*)0));
              }
          }
      }
  }

  median_skeletons_renderable::skeleton_buffer::handle
  median_skeletons_renderable::add( const std::string& skeleton_filename )
  {
    auto pair = m_skeletons.create();
    pair.second.skeleton.load( skeleton_filename );
    pair.second.dirty = true;
    return pair.first;
  }

  median_skeletons_renderable::skeleton_buffer::handle
  median_skeletons_renderable::    add( median_skeleton&& skeleton )
  {
    auto pair = m_skeletons.create();
    pair.second.skeleton = std::move( skeleton );
    pair.second.dirty = true;
    return pair.first;
  }

  void median_skeletons_renderable::remove( skeleton_buffer::handle h)
  {
    if( h.is_valid() )
      m_skeletons.get( h ).destroyed = true;
  }

   median_skeletons_renderable::storage&
   median_skeletons_renderable::get( skeleton_buffer::handle h)
   {
     return m_skeletons.get( h );
   }

   void
   median_skeletons_renderable::render_isolated_atoms( bool render )
   {
     m_render_isolated_atoms = render;
   }
   void
   median_skeletons_renderable::render_isolated_links( bool render )
   {
     m_render_isolated_links = render;
   }
   void
   median_skeletons_renderable::render_borders_junctions( bool render )
   {
     m_render_borders_junctions = render;
   }

   void
   median_skeletons_renderable::render_skeleton_points( bool render )
   {
     m_render_skeleton_points = render;
   }
   void
   median_skeletons_renderable::render_triangles( bool render )
   {
     m_render_triangles = render;
   }
   void
   median_skeletons_renderable::render_wireframe( bool render )
   {
     m_render_wireframe = render;
   }
   void
   median_skeletons_renderable::use_radii_colors( bool use )
   {
     m_use_radii_colors = use;
   }
   void
   median_skeletons_renderable::render_balls( bool render )
   {
     m_render_balls = render;
   }
END_MP_NAMESPACE
