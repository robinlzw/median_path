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

  typedef MP_NAMESPACE::skeleton_datastructure< uint32_t, 22, uint64_t, 44, uint64_t, 54 >
    skeleton;

  std::cout
    <<   "sizeof skeleton = " << sizeof( skeleton )
    << "\nsizeof atom     = " << sizeof( skeleton::atom )
    << "\nsizeof link     = " << sizeof( skeleton::link )
    << "\nsizeof face     = " << sizeof( skeleton::face )
    <<std::endl;

  skeleton myskeleton( 10, 0, 0 );

  myskeleton.add_atom_property<GO_NAMESPACE::gpu_vec4>( "color");

  return 0;
}
