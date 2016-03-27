/* Created on: Mar 1, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */

# include "../median-path/median_path.h"
# include "../median-path/median_skeleton.h"

int main( int argc, char* argv[] )
{
  median_path::median_skeleton test_skeleton;
  test_skeleton.load( std::string(argv[1] ) );
  test_skeleton.save( "test.median");
  return 0;
}
