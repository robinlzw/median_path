/* Created on: Mar 1, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */

# include "../median-path/median_path.h"
# include "../median-path/median_skeleton.h"
# include <graphics-origin/geometry/vec.h>
# include <iostream>

BEGIN_MP_NAMESPACE
struct atom
  : public GO_NAMESPACE::vec4 {

  real get_radius() const
  {
    return w;
  }

  GO_NAMESPACE::vec3 get_center() const
  {
    return GO_NAMESPACE::vec3{ x, y, z };
  }
};
END_MP_NAMESPACE

int main( int argc, char* argv[] )
{
  (void)argc;
  (void)argv;

  std::cout << "sizeof vec4 = " << sizeof( GO_NAMESPACE::vec4 )
      << "\n" << "sizeof atom = " << sizeof( MP_NAMESPACE::atom ) << std::endl;

  MP_NAMESPACE::median_skeleton skeleton;

  return 0;
}
