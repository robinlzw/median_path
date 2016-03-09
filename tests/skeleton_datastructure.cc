/*  Created on: Mar 6, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "test.h"
# include <graphics-origin/tools/log.h>

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

  static bool temp_atom_data_flags[ 10 ] = {
      false, false, false, false, false, false, false, false, false, false
  };
  static bool temp_link_data_flags[ 10 ] = {
      false, false, false, false, false, false, false, false, false, false
  };
  static bool temp_link_data_flags2[ 10 ] = {
      false, false, false, false, false, false, false, false, false, false
  };
  static bool temp_face_data_flags[ 10 ] = {
      false, false, false, false, false, false, false, false, false, false
  };
  struct temp_atom_data {
    temp_atom_data()
      : idx{ 0 }
    {}
    temp_atom_data&
    operator=( temp_atom_data&& other )
    {
      idx = other.idx;
      return *this;
    }
    ~temp_atom_data()
    {
      temp_atom_data_flags[ idx ] = true;
    }
    size_t idx;
  };

  struct temp_link_data {
    temp_link_data()
      : idx{ 0 }
    {}
    temp_link_data&
    operator=( temp_link_data&& other )
    {
      idx = other.idx;
      return *this;
    }
    ~temp_link_data()
    {
      temp_link_data_flags[ idx ] = true;
    }
    size_t idx;
  };

  struct temp_link_data2 {
    temp_link_data2()
      : idx{ 0 }
    {}
    temp_link_data2&
    operator=( temp_link_data2&& other )
    {
      idx = other.idx;
      return *this;
    }
    ~temp_link_data2()
    {
      temp_link_data_flags2[ idx ] = true;
    }
    size_t idx;
  };

  struct temp_face_data {
    temp_face_data()
      : idx{ 0 }
    {}
    temp_face_data&
    operator=( temp_face_data&& other )
    {
      idx = other.idx;
      return *this;
    }
    ~temp_face_data()
    {
      temp_face_data_flags[ idx ] = true;
    }
    size_t idx;
  };

  static void
  element_properties_destroyed()
  {
    {
      datastructure s( 20, 20, 20 );
      for( int i = 0; i < 10; ++ i )
        {
          s.create_atom();
          s.create_link();
          s.create_face();
        }
      s.add_atom_property<temp_atom_data>("temp_atom_data");
      s.add_link_property<temp_link_data>("temp_link_data");
      s.add_link_property<temp_link_data2>("temp_link_data2");
      s.add_face_property<temp_face_data>("temp_face_data");

      for( int i = 0; i < 10; ++ i )
        {
          s.m_atom_properties[0]->get<temp_atom_data>( i ).idx = i;
          s.m_link_properties[0]->get<temp_link_data>( i ).idx = i;
          s.m_link_properties[1]->get<temp_link_data2>( i ).idx = i;
          s.m_face_properties[0]->get<temp_face_data>( i ).idx = i;
        }
    }
    for( int i = 0; i < 10; ++ i )
      {
        BOOST_REQUIRE( temp_atom_data_flags[i] );
        BOOST_REQUIRE( temp_link_data_flags[i] );
        BOOST_REQUIRE( temp_link_data_flags2[i] );
        BOOST_REQUIRE( temp_face_data_flags[i] );
      }
  }




  extern test_suite* create_clear_test_suite();

  static test_suite*
  create_construction_test_suite()
  {
    test_suite* suite = BOOST_TEST_SUITE( "construction" );
    ADD_TEST_CASE( default_no_throw );
    ADD_TEST_CASE( capacity_no_throw );
    ADD_TEST_CASE( have_requested_capacities );
    return suite;
  }

  static test_suite*
  create_destruction_test_suite()
  {
    test_suite* suite = BOOST_TEST_SUITE( "destruction" );
//    ADD_TEST_CASE( no_memory_leaks ); // cannot find how to check it quickly inside a test
    ADD_TEST_CASE( element_properties_destroyed );
    return suite;
  }

  void add_skeleton_datastructure_test_suite()
  {
    test_suite* suite = BOOST_TEST_SUITE("SKELETON_DATASTRUCTURE");
    ADD_TO_SUITE( create_construction_test_suite );
    ADD_TO_SUITE( create_destruction_test_suite );
    ADD_TO_SUITE( create_clear_test_suite );
    ADD_TO_MASTER( suite );
  }


END_MP_NAMESPACE
