/*  Created on: Mar 10, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "test.h"
# include "../median-path/median_skeleton.h"
# include <graphics-origin/tools/log.h>
BEGIN_MP_NAMESPACE

  static void add_dont_throw()
  {
    median_skeleton s;
    for( int i = 0; i < 200; ++ i )
      {
        BOOST_REQUIRE_NO_THROW( s.add( vec4{ i, 2 *i, 3 * i, 4 * i } ) );
      }
  }

  static void atom_is_as_expected_just_after_creation()
  {
    median_skeleton s;
    for( int i = 0; i < 200; ++ i )
      {
        auto handle = s.add( vec4{ i, 2 *i, 3 * i, 4 * i } );
        auto& atom = s.get( handle );
        REAL_CHECK_CLOSE( atom.x, i, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.y, 2 * i, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.z, 3 * i, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.w, 4 * i, 1e-9, 1e-6 );
      }
  }

  static void atoms_remain_the_same_as_we_add_new_ones_without_reallocation()
  {
    median_skeleton s(200);
    median_skeleton::atom_handle handles[200];
    for( int i = 0; i < 200; ++ i )
      {
        handles[i] = s.add( vec4{ i, 2 *i, 3 * i, 4 * i } );
      }

    for( int i = 0; i < 200; ++ i )
      {
        auto& atom = s.get( handles[i] );
        REAL_CHECK_CLOSE( atom.x, i, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.y, 2 * i, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.z, 3 * i, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.w, 4 * i, 1e-9, 1e-6 );
      }
  }

  static void atoms_remain_the_same_as_we_add_new_ones()
  {
    median_skeleton s;
    median_skeleton::atom_handle handles[200];
    for( int i = 0; i < 200; ++ i )
      {
        handles[i] = s.add( vec4{ i, 2 *i, 3 * i, 4 * i } );
      }

    for( int i = 0; i < 200; ++ i )
      {
        auto& atom = s.get( handles[i] );
        REAL_CHECK_CLOSE( atom.x, i, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.y, 2 * i, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.z, 3 * i, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.w, 4 * i, 1e-9, 1e-6 );
      }
  }

  static void atoms_remain_at_the_same_location_without_reallocation()
  {
    median_skeleton s(200);
    median_skeleton::atom_handle handles[200];
    size_t addresses[200];
    for( int i = 0; i < 200; ++ i )
      {
        handles[i] = s.add( vec4{ i, 2 *i, 3 * i, 4 * i } );
        addresses[i] = reinterpret_cast< size_t >( &s.get( handles[i] ) );
      }

    for( int i = 0; i < 200; ++ i )
      {
        BOOST_REQUIRE_EQUAL( reinterpret_cast< size_t >( &s.get( handles[ i ] ) ), addresses[i] );
      }
  }

  static void remove_last_atom()
  {
    median_skeleton s;
    median_skeleton::atom_handle handles[200];
    for( int i = 0; i < 200; ++ i )
      {
        handles[i] = s.add( vec4{ i, 2 *i, 3 * i, 4 * i } );
      }

    s.remove( handles[199] );
    BOOST_REQUIRE_EQUAL( s.get_number_of_atoms(), 199 );
    BOOST_REQUIRE( !s.is_valid( handles[199] ) );
    for( int i = 0; i < 199; ++ i )
      {
        auto& atom = s.get( handles[i] );
        REAL_CHECK_CLOSE( atom.x, i, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.y, 2 * i, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.z, 3 * i, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.w, 4 * i, 1e-9, 1e-6 );
      }
  }

  static void remove_one_atom()
  {
    median_skeleton s;
    median_skeleton::atom_handle handles[200];
    for( int i = 0; i < 200; ++ i )
      {
        handles[i] = s.add( vec4{ i, 2 *i, 3 * i, 4 * i } );
      }

    s.remove( handles[100] );
    BOOST_REQUIRE_EQUAL( s.get_number_of_atoms(), 199 );
    BOOST_REQUIRE( !s.is_valid( handles[100] ) );
    for( int i = 0; i < 200; ++ i )
      {
        if( i == 100 ) continue;
        auto& atom = s.get( handles[i] );
        REAL_CHECK_CLOSE( atom.x, i, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.y, 2 * i, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.z, 3 * i, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.w, 4 * i, 1e-9, 1e-6 );
      }
  }

  static void indices_are_incremental_without_remove()
  {
    median_skeleton s;
    median_skeleton::atom_handle handles[200];
    for( int i = 0; i < 200; ++ i )
      {
        handles[i] = s.add( vec4{ i, 2 *i, 3 * i, 4 * i } );
      }

    for( int i = 0; i < 200; ++ i )
      {
        auto& atom = s.get( handles[i] );
        BOOST_REQUIRE_EQUAL( s.get_index( atom ), i );
        BOOST_REQUIRE_EQUAL( s.get_index( handles[ i ] ), i );
      }
  }


  static void remove_even_atoms()
  {
    median_skeleton s;
    median_skeleton::atom_handle handles[20];
    for( int i = 0; i < 20; ++ i )
      {
        handles[i] = s.add( vec4{ i, 2 *i, 3 * i, 4 * i } );
      }
    for( int i = 0; i < 10; ++ i )
      {
        s.remove( handles[ 2 * i ] );
      }
    BOOST_REQUIRE_EQUAL( s.get_number_of_atoms(), 10 );
    for( int i = 0; i < 10; ++ i )
      {
        BOOST_REQUIRE( !s.is_valid( handles[2 * i] ) );
        auto& atom = s.get( handles[2 * i + 1] );
        REAL_CHECK_CLOSE( atom.x,     (2 * i + 1), 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.y, 2 * (2 * i + 1), 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.z, 3 * (2 * i + 1), 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.w, 4 * (2 * i + 1), 1e-9, 1e-6 );
      }
  }

  static void remove_even_atoms_by_filter()
  {
    median_skeleton s;
    median_skeleton::atom_handle handles[200];
    for( int i = 0; i < 200; ++ i )
      {
        handles[i] = s.add( vec4{ i, 2 *i, 3 * i, 4 * i } );
      }

    s.remove( [&s](median_skeleton::atom& a)->bool
      {
        return !(s.get_index(a) & 1);
      });

    BOOST_REQUIRE_EQUAL( s.get_number_of_atoms(), 100 );
    for( int i = 0; i < 100; ++ i )
      {
        BOOST_CHECK( !s.is_valid( handles[2 * i] ) );
        auto& atom = s.get( handles[2 * i + 1] );
        REAL_CHECK_CLOSE( atom.x,     (2 * i + 1), 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.y, 2 * (2 * i + 1), 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.z, 3 * (2 * i + 1), 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.w, 4 * (2 * i + 1), 1e-9, 1e-6 );
      }
  }

  static void process_atoms()
  {
    median_skeleton s;
    median_skeleton::atom_handle handles[200];
    for( int i = 0; i < 200; ++ i )
      {
        handles[i] = s.add( vec4{ i, 2 *i, 3 * i, 4 * i } );
      }

    s.process( [&s](median_skeleton::atom& a )
     {
        a.y /= 2.0;
        a.z /= 3.0;
        a.w /= 4.0;
     });
    for( int i = 0; i < 200; ++ i )
      {
        auto& atom = s.get( handles[i] );
        REAL_CHECK_CLOSE( atom.x, i, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.y, i, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.z, i, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.w, i, 1e-9, 1e-6 );
      }

  }

  test_suite* atom_management_test_suite()
  {
    test_suite* suite = BOOST_TEST_SUITE( "atom_management" );
    ADD_TEST_CASE( add_dont_throw );
    ADD_TEST_CASE( atom_is_as_expected_just_after_creation );
    ADD_TEST_CASE( indices_are_incremental_without_remove );
    ADD_TEST_CASE( atoms_remain_the_same_as_we_add_new_ones_without_reallocation );
    ADD_TEST_CASE( atoms_remain_the_same_as_we_add_new_ones );
    ADD_TEST_CASE( atoms_remain_at_the_same_location_without_reallocation );
    ADD_TEST_CASE( remove_last_atom );
    ADD_TEST_CASE( remove_one_atom );
    ADD_TEST_CASE( remove_even_atoms );
    ADD_TEST_CASE( remove_even_atoms_by_filter );
    ADD_TEST_CASE( process_atoms );
    return suite;
  }

END_MP_NAMESPACE
