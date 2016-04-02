/*  Created on: Mar 29, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */

# ifndef MEDIAN_PATH_SKELETON_RENDERABLE_H_
# define MEDIAN_PATH_SKELETON_RENDERABLE_H_
# include "../median-path/median_skeleton.h"

# include <graphics-origin/application/renderable.h>
# include <graphics-origin/tools/tight_buffer_manager.h>

BEGIN_MP_NAMESPACE

  /**
   *
   */
  class median_skeletons_renderable
    : public graphics_origin::application::renderable {

    enum { balls_vbo, colors_vbo, isolated_vertices_ibo, links_ibo, isolated_links_ibo, faces_ibo, number_of_buffers };
  public:

    struct storage {
      median_skeleton skeleton;
      unsigned int buffer_ids[number_of_buffers ];
      unsigned int vao;
      median_skeleton::atom_index number_of_isolated_atoms;
      median_skeleton::link_index number_of_isolated_links;
      bool dirty;
      bool active;
      bool destroyed;
      storage& operator=( storage&& other );
      storage();
    };

  private:
    typedef graphics_origin::tools::tight_buffer_manager<
        storage,
        uint32_t,
        22 > skeleton_buffer;

  public:
    typedef skeleton_buffer::handle handle;

    median_skeletons_renderable(
        graphics_origin::application::shader_program_ptr program,
        graphics_origin::application::shader_program_ptr isolated);
    ~median_skeletons_renderable();

    handle
    add( const std::string& skeleton_filename );

    handle
    add( median_skeleton&& skeleton );

    void remove( skeleton_buffer::handle h);

    storage& get( skeleton_buffer::handle h);

    void render_isolated_atoms( bool render );
    void render_isolated_links( bool render );

  private:
    void update_gpu_data() override;
    void do_render() override;
    void remove_gpu_data() override;

    skeleton_buffer m_skeletons;
    graphics_origin::application::shader_program_ptr m_isolated_program;
    bool m_render_isolated_atoms;
    bool m_render_isolated_links;
  };


END_MP_NAMESPACE
# endif
