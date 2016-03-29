/*  Created on: Mar 29, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "skeletons_renderable.h"

# include <graphics-origin/application/gl_window_renderer.h>
# include <graphics-origin/application/gl_helper.h>
# include <graphics-origin/geometry/vec.h>

# include <GL/glew.h>
BEGIN_MP_NAMESPACE
  median_skeletons_renderable::storage::storage( median_skeleton&& skeleton )
    : skeleton{ std::move( skeleton ) }, buffer_ids{ 0, 0, 0, 0 }, dirty{ true },
      active{false}
  {}

  median_skeletons_renderable::storage&
  median_skeletons_renderable::storage::operator=( storage&& other )
  {
    skeleton = std::move( other.skeleton );
    buffer_ids[ 0 ] = other.buffer_ids[ 0 ];
    buffer_ids[ 1 ] = other.buffer_ids[ 1 ];
    buffer_ids[ 2 ] = other.buffer_ids[ 2 ];
    buffer_ids[ 3 ] = other.buffer_ids[ 3 ];
    dirty = other.dirty;
    active = other.active;
    return *this;
  }

  median_skeletons_renderable::storage::storage()
    : buffer_ids{0,0,0,0}, dirty{true}, active{ false }
  {}


  median_skeletons_renderable::median_skeletons_renderable(
      graphics_origin::application::shader_program_ptr program )
  {
    m_model = gpu_mat4(1.0);
    m_program = program;
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
        glcheck(glDeleteBuffers( number_of_buffers, data->buffer_ids));
      }
  }

  void
  median_skeletons_renderable::update_gpu_data()
  {
    auto size = m_skeletons.get_size();
    auto data = m_skeletons.data();
    std::vector<uint32_t> indices;
    std::vector<gpu_vec4> colors;
    for( decltype(size) i = 0; i < size; ++ i, ++data )
      {
        if( data->dirty )
          {
            if( !data->buffer_ids[0] )
              {
                glcheck(glGenBuffers( number_of_buffers, data->buffer_ids));
              }

            const median_skeleton::atom_index nbatoms = data->skeleton.get_number_of_atoms();

            glcheck(glBindBuffer( GL_ARRAY_BUFFER, data->buffer_ids[balls_vbo]));
            glcheck(glBufferData( GL_ARRAY_BUFFER, sizeof(median_skeleton::atom) * nbatoms,
              &data->skeleton.get_atom_by_index( 0 ), GL_STATIC_DRAW ));

            real minr, maxr;
            data->skeleton.compute_minmax_radii( minr, maxr );

            colors.resize( nbatoms );
            # pragma omp parallel for
            for( median_skeleton::atom_index j = 0; j < nbatoms; ++ j )
              {
                colors[ j ] = gpu_vec4(graphics_origin::get_color( data->skeleton.get_atom_by_index( j ).w, minr, maxr ), 1.0 );
              }
            glcheck(glBindBuffer( GL_ARRAY_BUFFER, data->buffer_ids[colors_vbo]));
            glcheck(glBufferData( GL_ARRAY_BUFFER, sizeof(gpu_vec4) * nbatoms, colors.data(), GL_STATIC_DRAW));

            const median_skeleton::link_index nblinks = data->skeleton.get_number_of_links();
            indices.resize( nblinks * 2 );
            # pragma omp parallel for
            for( median_skeleton::link_index j = 0; j < nblinks; ++ j )
              {
                const auto& link = data->skeleton.get_link_by_index( j );
                size_t offset = j * 2;
                indices[ offset    ] = data->skeleton.get_index( link.h1 );
                indices[ offset + 1] = data->skeleton.get_index( link.h2 );
              }

            glcheck(glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, data->buffer_ids[links_ibo]));
            glcheck(glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW));

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

            data[size].dirty = false;
          }
      }
  }

  void
  median_skeletons_renderable::do_render()
  {
//    glcheck(glUniformMatrix4fv( m_program->get_uniform_location()))
  }


END_MP_NAMESPACE
