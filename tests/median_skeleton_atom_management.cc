/*  Created on: Mar 10, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "test.h"
# include "../median-path/median_skeleton.h"
# include <graphics-origin/tools/log.h>

# include <fstream>
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

  static void memory_reused_instead_of_reallocation()
  {
    median_skeleton s( 200 );
    median_skeleton::atom_handle handles[300];

    BOOST_CHECK_EQUAL( s.get_atoms_capacity(), 200 );
    for( int i = 0; i < 200; ++ i )
      {
        handles[i] = s.add( vec4{ i, i, i, i } );
      }

    BOOST_CHECK_EQUAL( s.get_atoms_capacity(), s.get_number_of_atoms());
    for( int i = 0; i < 100; ++ i )
      {
        s.remove( handles[ 2 * i] );
      }

    BOOST_CHECK_EQUAL( s.get_number_of_atoms(), 100 );
    for( int i = 200; i < 300; ++ i )
      {
        handles[i] = s.add( vec4{ i, i, i, i } );
      }
    BOOST_CHECK_EQUAL( s.get_number_of_atoms(), 200 );
    BOOST_CHECK_EQUAL( s.get_atoms_capacity(), 200 );
  }

  static void filter_move_atoms_as_expected()
  {
    median_skeleton s( 10 );
    median_skeleton::atom_handle handles[10];
    for( int i = 0; i < 10; ++ i )
      {
        // atom #i is a_i = {i,i,i,i}
        handles[i] = s.add( vec4{ i, i, i, i } );
      }
    // all odd atoms are removed,
    s.remove( [&s]( median_skeleton::atom& atom ){ return s.get_index( atom ) & 1; }, true );
    BOOST_REQUIRE_EQUAL( s.get_number_of_atoms(), 5 );
    // a_0    a_2    a_4
    //    a_8    a_6

    // check handles and handles --> atoms
    for( int i = 0; i < 5; ++ i )
      {
        BOOST_REQUIRE( s.is_valid( handles[i * 2] ) );
        BOOST_REQUIRE( !s.is_valid( handles[i * 2 + 1 ] ) );
        median_skeleton::atom& atom = s.get( handles[ 2 * i ] );
        REAL_CHECK_CLOSE( atom.x, 2*i, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.y, 2*i, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.z, 2*i, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.w, 2*i, 1e-9, 1e-6 );
      }

    // check handles --> atoms (useless for now, but may be important if the handle buffer change)
    BOOST_REQUIRE_EQUAL( s.get_index( handles[0] ), median_skeleton::atom_index(0) );
    BOOST_REQUIRE_EQUAL( s.get_index( handles[8] ), median_skeleton::atom_index(1) );
    BOOST_REQUIRE_EQUAL( s.get_index( handles[2] ), median_skeleton::atom_index(2) );
    BOOST_REQUIRE_EQUAL( s.get_index( handles[6] ), median_skeleton::atom_index(3) );
    BOOST_REQUIRE_EQUAL( s.get_index( handles[4] ), median_skeleton::atom_index(4) );

    // check index --> handles
    BOOST_REQUIRE_EQUAL( s.get_handle(s.get_atom_by_index(0)), handles[0] );
    BOOST_REQUIRE_EQUAL( s.get_handle(s.get_atom_by_index(1)), handles[8] );
    BOOST_REQUIRE_EQUAL( s.get_handle(s.get_atom_by_index(2)), handles[2] );
    BOOST_REQUIRE_EQUAL( s.get_handle(s.get_atom_by_index(3)), handles[6] );
    BOOST_REQUIRE_EQUAL( s.get_handle(s.get_atom_by_index(4)), handles[4] );
  }

  static void filter_move_atoms_as_expected_with_an_initial_odd_number()
  {
    median_skeleton s( 11 );
    median_skeleton::atom_handle handles[11];
    // a0 a1 a2 a3 a4 a5 a6 a7 a8 a9 a10
    for( int i = 0; i < 11; ++ i )
      {
        // atom #i is a_i = {i,i,i,i}
        handles[i] = s.add( vec4{ i, i, i, i } );
      }
    // all even atoms are removed,
    s.remove( [&s]( median_skeleton::atom& atom ){ return !(s.get_index( atom ) & 1); }, true );

    // a9 a1 a7 a3 a5
    BOOST_REQUIRE_EQUAL( s.get_number_of_atoms(), 5 );

    // check handles and handles --> atoms
    for( int i = 0; i < 5; ++ i )
      {
        BOOST_REQUIRE( !s.is_valid( handles[i * 2] ) );
        BOOST_REQUIRE( s.is_valid( handles[i * 2 + 1 ] ) );
        median_skeleton::atom& atom = s.get( handles[ 2 * i + 1] );
        REAL_CHECK_CLOSE( atom.x, 2*i + 1, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.y, 2*i + 1, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.z, 2*i + 1, 1e-9, 1e-6 );
        REAL_CHECK_CLOSE( atom.w, 2*i + 1, 1e-9, 1e-6 );
      }

    // check handles --> atoms (useless for now, but may be important if the handle buffer change)
    BOOST_REQUIRE_EQUAL( s.get_index( handles[9] ), median_skeleton::atom_index(0) );
    BOOST_REQUIRE_EQUAL( s.get_index( handles[1] ), median_skeleton::atom_index(1) );
    BOOST_REQUIRE_EQUAL( s.get_index( handles[7] ), median_skeleton::atom_index(2) );
    BOOST_REQUIRE_EQUAL( s.get_index( handles[3] ), median_skeleton::atom_index(3) );
    BOOST_REQUIRE_EQUAL( s.get_index( handles[5] ), median_skeleton::atom_index(4) );

    // check index --> handles
    BOOST_REQUIRE_EQUAL( s.get_handle(s.get_atom_by_index(0)), handles[9] );
    BOOST_REQUIRE_EQUAL( s.get_handle(s.get_atom_by_index(1)), handles[1] );
    BOOST_REQUIRE_EQUAL( s.get_handle(s.get_atom_by_index(2)), handles[7] );
    BOOST_REQUIRE_EQUAL( s.get_handle(s.get_atom_by_index(3)), handles[3] );
    BOOST_REQUIRE_EQUAL( s.get_handle(s.get_atom_by_index(4)), handles[5] );
  }

  static void load_balls_file()
  {
    median_skeleton s;

    {
      std::ofstream temp( "temp.balls" );
      temp << "5\n"
           << "0 0 0 0 1 1 1 1 2 \n"
           << "2 2 2  //end of the third ball\n"
           << "3 3 3 3 4 4 4 4 \n"
           << "5 5 5 5 //this is not read";
      temp.close();
    }

    BOOST_REQUIRE( s.load( "temp.balls") );
    BOOST_REQUIRE_EQUAL( s.get_number_of_atoms(), 5 );
    BOOST_REQUIRE_EQUAL( s.get_number_of_links(), 0 );
    BOOST_REQUIRE_EQUAL( s.get_number_of_faces(), 0 );

    for( int i = 0; i < 5; ++ i )
      {
        auto& atom = s.get_atom_by_index( i );
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
    ADD_TEST_CASE( memory_reused_instead_of_reallocation );
    ADD_TEST_CASE( filter_move_atoms_as_expected );
    ADD_TEST_CASE( filter_move_atoms_as_expected_with_an_initial_odd_number );
    ADD_TEST_CASE( load_balls_file );
    return suite;
  }

END_MP_NAMESPACE
