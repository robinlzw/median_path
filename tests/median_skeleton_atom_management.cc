/*  Created on: Mar 10, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "test.h"
# include "../median-path/median_skeleton.h"
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

  test_suite* atom_management_test_suite()
  {
    test_suite* suite = BOOST_TEST_SUITE( "atom_management" );
    ADD_TEST_CASE( add_dont_throw );
    ADD_TEST_CASE( atom_is_as_expected_just_after_creation );
    ADD_TEST_CASE( atoms_remain_the_same_as_we_add_new_ones_without_reallocation );
    ADD_TEST_CASE( atoms_remain_the_same_as_we_add_new_ones );
    return suite;
  }

END_MP_NAMESPACE
