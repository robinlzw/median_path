/* Created on: Mar 4, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# include <median-path/median_skeleton.h>
# include <graphics-origin/tools/log.h>

BEGIN_MP_NAMESPACE

  median_skeleton::median_skeleton(
      uint32_t atom_capacity,
      uint64_t link_capacity,
      uint64_t face_capacity)
    : m_impl{ new datastructure{ atom_capacity, link_capacity, face_capacity } }
  {
    LOG( debug, "TODO: load default atom, link and face property");
  }

  median_skeleton::median_skeleton( median_skeleton&& other )
    : m_impl{ other.m_impl }
  {
    other.m_impl = new datastructure{};
  }

  median_skeleton::median_skeleton( const std::string& filename )
  {
    load( filename );
  }

  median_skeleton::~median_skeleton()
  {
    delete m_impl;
  }

  void median_skeleton::clear(
      uint32_t atom_capacity,
      uint64_t link_capacity,
      uint64_t face_capacity )
  {
  }

END_MP_NAMESPACE
