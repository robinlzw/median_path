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

static void grow_buffers()
{
  datastructure s(10, 20, 30);
  s.clear( 100, 200, 300 );
  BOOST_CHECK_EQUAL( 100, s.m_atoms_capacity );
  BOOST_CHECK_EQUAL( 200, s.m_links_capacity );
  BOOST_CHECK_EQUAL( 300, s.m_faces_capacity );

  for( size_t i = 0; i < s.m_atoms_capacity; ++i )
    {
      BOOST_REQUIRE_NO_THROW( ++s.m_atoms[ i ].x );
      BOOST_REQUIRE_NO_THROW( ++s.m_atom_handles[ i ].counter);
      BOOST_REQUIRE_NO_THROW( ++s.m_atom_index_to_handle_index[ i ]);
    }
  for( size_t i = 0; i < s.m_links_capacity; ++i )
    {
      BOOST_REQUIRE_NO_THROW( ++s.m_links[ i ].h1.counter );
      BOOST_REQUIRE_NO_THROW( ++s.m_link_handles[ i ].counter);
      BOOST_REQUIRE_NO_THROW( ++s.m_link_index_to_handle_index[ i ]);
    }
  for( size_t i = 0; i < s.m_faces_capacity; ++i )
    {
      BOOST_REQUIRE_NO_THROW( ++s.m_faces[ i ].atoms[2].counter );
      BOOST_REQUIRE_NO_THROW( ++s.m_face_handles[ i ].counter);
      BOOST_REQUIRE_NO_THROW( ++s.m_face_index_to_handle_index[ i ]);
    }
}

test_suite* create_clear_test_suite()
{
  test_suite* suite = BOOST_TEST_SUITE( "clear" );
  ADD_TEST_CASE( do_not_throw_on_allocated );
  ADD_TEST_CASE( do_not_throw_on_empty );
  ADD_TEST_CASE( do_not_shrink );
  ADD_TEST_CASE( grow_buffers );
  return suite;
}


END_MP_NAMESPACE
