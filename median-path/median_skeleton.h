/* Created on: Mar 1, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# ifndef MEDIAN_PATH_MEDIAN_SKELETON_H_
# define MEDIAN_PATH_MEDIAN_SKELETON_H_

# include "median_path.h"
# include <graphics-origin/tools/tight_buffer_manager.h>
# include <graphics-origin/geometry/vec.h>



BEGIN_MP_NAMESPACE

  class median_skeleton {
  public:
    struct atom
      : public GO_NAMESPACE::vec4 {
      atom& operator=( atom&& other )
      {
        GO_NAMESPACE::vec4::operator=( std::move(other) );
        return *this;
      }

      atom()
        : GO_NAMESPACE::vec4{}
      {}
    };




  private:
    GO_NAMESPACE::tools::tight_buffer_manager<
      atom,
      uint32_t,
      22 > m_atoms;
  };

END_MP_NAMESPACE
# endif 
