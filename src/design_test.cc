/* Created on: Mar 1, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */

# include "../median-path/median_path.h"
# include "../median-path/median_skeleton.h"
# include <graphics-origin/geometry/vec.h>
# include <iostream>
# include <list>
# include <stdint.h>

int main( int argc, char* argv[] )
{
  (void)argc;
  (void)argv;

  std::cout << "sizeof vec4 = " << sizeof( GO_NAMESPACE::vec4 )
      << "\nsizeof atom = " << sizeof( MP_NAMESPACE::atom )
      << "\nsizeof link = " << sizeof( MP_NAMESPACE::link )
      << std::endl;


  MP_NAMESPACE::median_skeleton skeleton;

  return 0;
}
