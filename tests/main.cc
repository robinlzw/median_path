/*  Created on: Mar 6, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */

# include "test.h"
# include <graphics-origin/tools/log.h>
BEGIN_MP_NAMESPACE

  extern void add_skeleton_datastructure_test_suite();
  extern void add_median_skeleton_test_suite();

  static bool
  initialize_tests()
  {
    GO_NAMESPACE::tools::init_log("fulltest.log");
    master_test_suite().p_name.value = "Median Path test suite";
    add_skeleton_datastructure_test_suite();
    add_median_skeleton_test_suite();
    return true;
  }

END_MP_NAMESPACE

int main( int argc, char* argv[] )
{
  auto res = ::boost::unit_test::unit_test_main(
    &MP_NAMESPACE::initialize_tests,
    argc,
    argv );
  GO_NAMESPACE::tools::flush_log();
  return res;
}
