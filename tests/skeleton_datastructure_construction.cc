/* Created on: Mar 9, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# include "test.h"
BEGIN_MP_NAMESPACE

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

    for( size_t i = 0; i < 10; ++ i )
      BOOST_REQUIRE_NO_THROW( s.m_atoms[i].x = s.m_atoms[i].y + s.m_atoms[i].w );
    for( size_t i = 0; i < 100; ++ i )
      BOOST_REQUIRE_NO_THROW( s.m_links[i].h1.index = s.m_links[i].h2.index * 2 );
    for( size_t i = 0; i < 1000; ++ i )
      BOOST_REQUIRE_NO_THROW( s.m_faces[i].atoms[2].is_valid() ? s.m_faces[i].links[2].index = 1 : s.m_faces[i].links[2].index = 0);
  }

  static void
  empty_is_as_expected()
  {
    datastructure s;
    BOOST_REQUIRE( s.m_atoms == nullptr );
    BOOST_REQUIRE( s.m_atom_index_to_handle_index == nullptr );
    BOOST_REQUIRE( s.m_atom_handles == nullptr );
    BOOST_REQUIRE( s.m_atom_properties.empty() );
    BOOST_REQUIRE_EQUAL( s.m_atoms_capacity, 0 );
    BOOST_REQUIRE_EQUAL( s.m_atoms_size, 0 );
    BOOST_REQUIRE_EQUAL( s.m_atoms_next_free_handle_slot, 0 );

    BOOST_REQUIRE( s.m_links == nullptr );
    BOOST_REQUIRE( s.m_link_index_to_handle_index == nullptr );
    BOOST_REQUIRE( s.m_link_handles == nullptr );
    BOOST_REQUIRE( s.m_link_properties.empty() );
    BOOST_REQUIRE_EQUAL( s.m_links_capacity, 0 );
    BOOST_REQUIRE_EQUAL( s.m_links_size, 0 );
    BOOST_REQUIRE_EQUAL( s.m_links_next_free_handle_slot, 0 );

    BOOST_REQUIRE( s.m_faces == nullptr );
    BOOST_REQUIRE( s.m_face_index_to_handle_index == nullptr );
    BOOST_REQUIRE( s.m_face_handles == nullptr );
    BOOST_REQUIRE( s.m_face_properties.empty() );
    BOOST_REQUIRE_EQUAL( s.m_faces_capacity, 0 );
    BOOST_REQUIRE_EQUAL( s.m_faces_size, 0 );
    BOOST_REQUIRE_EQUAL( s.m_faces_next_free_handle_slot, 0 );
  }

  test_suite* create_construction_test_suite()
  {
    test_suite* suite = BOOST_TEST_SUITE( "construction" );
    ADD_TEST_CASE( default_no_throw );
    ADD_TEST_CASE( capacity_no_throw );
    ADD_TEST_CASE( have_requested_capacities );
    ADD_TEST_CASE( empty_is_as_expected );
    return suite;
  }
END_MP_NAMESPACE
