/* Created on: Mar 9, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# include "test.h"

BEGIN_MP_NAMESPACE

static void do_not_throw_on_allocated()
{
  datastructure s(10, 20, 30);
  BOOST_REQUIRE_NO_THROW( s.clear( 5, 10, 15 ) );
  BOOST_REQUIRE_NO_THROW( s.clear(100,200,300 ) );
}

static void do_not_throw_on_empty()
{
  datastructure s;
  BOOST_REQUIRE_NO_THROW( s.clear( 5, 10, 15 ) );
}

static void do_not_shrink()
{
  datastructure s(10, 20, 30);
  s.clear( 5, 10, 15 );
  BOOST_CHECK_EQUAL( 10, s.m_atoms_capacity );
  BOOST_CHECK_EQUAL( 20, s.m_links_capacity );
  BOOST_CHECK_EQUAL( 30, s.m_faces_capacity );
}

test_suite* create_clear_test_suite()
{
  test_suite* suite = BOOST_TEST_SUITE( "clear" );
  ADD_TEST_CASE( do_not_throw_on_allocated );
  ADD_TEST_CASE( do_not_throw_on_empty );
  ADD_TEST_CASE( do_not_shrink );
  return suite;
}


END_MP_NAMESPACE
