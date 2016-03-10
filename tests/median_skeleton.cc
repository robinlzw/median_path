/*  Created on: Mar 10, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "test.h"
BEGIN_MP_NAMESPACE

 extern test_suite* atom_management_test_suite();
 void add_median_skeleton_test_suite()
 {
   test_suite* suite = BOOST_TEST_SUITE( "MEDIAN_SKELETON" );
   ADD_TO_SUITE( atom_management_test_suite );
   ADD_TO_MASTER( suite );
 }

END_MP_NAMESPACE
