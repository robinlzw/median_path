/*  Created on: Mar 10, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "test.h"

BEGIN_MP_NAMESPACE

  static void create_on_empty_skeleton_dont_throw()
  {
    datastructure s;
    BOOST_REQUIRE_NO_THROW(s.create_atom());
    BOOST_REQUIRE_NO_THROW(s.create_link());
    BOOST_REQUIRE_NO_THROW(s.create_face());
  }

  static void increase_size()
  {
    datastructure s;
    for( size_t i = 0; i < 100; ++ i )
      {
        s.create_atom();
        BOOST_REQUIRE_EQUAL( s.m_atoms_size, i + 1 );
        s.create_link();
        BOOST_REQUIRE_EQUAL( s.m_links_size, i + 1 );
        s.create_face();
        BOOST_REQUIRE_EQUAL( s.m_faces_size, i + 1 );
      }
  }

  static void give_access_to_new_elements()
  {
    datastructure s;
    auto first = s.create_atom();
    first.second.x = 1.0;
    auto second = s.create_atom();
    second.second.x = 2.0;
    REAL_CHECK_CLOSE( first.second.x, 1.0, 1e-9, 1e-6 );
    REAL_CHECK_CLOSE( second.second.x, 2.0, 1e-9, 1e-6 );
  }

  static void handle_points_to_the_new_element()
  {
    datastructure s;
    for( size_t i = 0; i < 200; ++ i )
      {
        {
          auto pair = s.create_atom();
          BOOST_REQUIRE_EQUAL( &pair.second, &s.get( pair.first ) );
        }
        {
          auto pair = s.create_link();
          BOOST_REQUIRE_EQUAL( &pair.second, &s.get( pair.first ) );
        }
        {
          auto pair = s.create_face();
          BOOST_REQUIRE_EQUAL( &pair.second, &s.get( pair.first ) );
        }
      }
  }

  static void new_element_has_the_expected_handle()
  {
    datastructure s;
    for( size_t i = 0; i < 200; ++ i )
      {
        {
          auto pair = s.create_atom();
          BOOST_REQUIRE_EQUAL( pair.first, s.get_handle( pair.second ) );
        }
        {
          auto pair = s.create_link();
          BOOST_REQUIRE_EQUAL( pair.first, s.get_handle( pair.second ) );
        }
        {
          auto pair = s.create_face();
          BOOST_REQUIRE_EQUAL( pair.first, s.get_handle( pair.second ) );
        }
      }
  }

  static void new_element_has_expected_property()
  {
    datastructure s;
    s.add_atom_property<size_t>( "integer" );

    for( size_t i = 0; i < 200; ++ i )
      {
        auto pair = s.create_atom();
        s.m_atom_properties[0]->get<size_t>( i ) = i;
      }

    for( size_t i = 0; i < 200; ++ i )
      {
        BOOST_REQUIRE_EQUAL( s.m_atom_properties[0]->get<size_t>( i ), i );
      }
  }

  test_suite* create_element_test_suite()
  {
    test_suite* suite = BOOST_TEST_SUITE( "destruction" );
    ADD_TEST_CASE( create_on_empty_skeleton_dont_throw );
    ADD_TEST_CASE( increase_size );
    ADD_TEST_CASE( give_access_to_new_elements );
    ADD_TEST_CASE( handle_points_to_the_new_element );
    ADD_TEST_CASE( new_element_has_the_expected_handle );
    ADD_TEST_CASE( new_element_has_expected_property );
    return suite;
  }

END_MP_NAMESPACE
