/*  Created on: Mar 6, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "test.h"
# include <graphics-origin/tools/log.h>
# include "../median-path/detail/skeleton_datastructure.h"

BEGIN_MP_NAMESPACE

  typedef skeleton_datastructure<
    uint32_t, 22,
    uint64_t, 44,
    uint64_t, 54 > datastructure;

  static void
  default_no_throw()
  {
    BOOST_REQUIRE_NO_THROW( datastructure s );
  }

  static void
  capacity_no_throw()
  {
    BOOST_REQUIRE_NO_THROW( datastructure s( 10, 100, 1000 ) );
  }

  static void
  have_requested_capacities()
  {
    datastructure s( 10, 100, 1000 );
    BOOST_CHECK_EQUAL(   10, s.m_atoms_capacity );
    BOOST_CHECK_EQUAL(  100, s.m_links_capacity );
    BOOST_CHECK_EQUAL( 1000, s.m_faces_capacity );

  }

  static test_suite*
  create_construction_test_suite()
  {
    test_suite* suite = BOOST_TEST_SUITE( "construction" );
    ADD_TEST_CASE( default_no_throw );
    ADD_TEST_CASE( capacity_no_throw );
    ADD_TEST_CASE( have_requested_capacities );
    return suite;
  }

  void add_skeleton_datastructure_test_suite()
  {
    test_suite* suite = BOOST_TEST_SUITE("SKELETON_DATASTRUCTURE");
    ADD_TO_SUITE( create_construction_test_suite );
    ADD_TO_MASTER( suite );
  }


END_MP_NAMESPACE
