#**********************
# cmake configuration *
#*******************************************************************************
cmake_minimum_required( VERSION 2.8.11 )

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# Graphics-Origin project
project( Median-Path )
set( Median-Path_VERSION_MAJOR 1 )
set( Median-Path_VERSION_MINOR 0 )

if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE "Release" 
    CACHE STRING "Choose the type of build, options are: Debug Release." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

#****************************
# directories configuration *
#*******************************************************************************
set( MP_HEADERS_IN_DIR "${CMAKE_SOURCE_DIR}" )
set( MP_SOURCES_IN_DIR "${CMAKE_SOURCE_DIR}/lib" )
set( MP_TEST_IN_DIR "${CMAKE_SOURCE_DIR}/tests" )
set( MP_APPLICATIONS_IN_DIR "${CMAKE_SOURCE_DIR}/src" )
set( MP_EXTLIBS_DIR "${CMAKE_SOURCE_DIR}/extlibs" )

set( MP_EXECUTABLE_OUT_DIR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin" )
set( MP_LIBRARY_OUT_DIR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib" )
set( MP_APPLICATIONS_OUT_DIR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/apps" )
set( MP_TEST_OUT_DIR "${MP_APPLICATIONS_OUT_DIR}" )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MP_LIBRARY_OUT_DIR} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${MP_LIBRARY_OUT_DIR} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MP_EXECUTABLE_OUT_DIR} )

#*********************
# projection options *
#*******************************************************************************
option(
  MP_USE_CGAL
  "Use CGAL libraries"
  ON)
  
option(
  MP_USE_THRUST
  "Use Cuda/Thrust library"
  ON)
  
option(
  MP_USE_CUDA
  "Set to ON to use CUDA libraries. If set to false, some functionnalities won't be available"
  ON)  
  
option(
  MP_USE_GO_APPLICATION
  "Set to ON to use GraphicsOrigin::application. If set to false, only command line tools would be available."
  ON)

option(
  MP_USE_GO_GEOMETRY_WITH_CUDA
  "Set to ON if GraphicsOrigin::geometry uses CUDA. If set to false, some functionnalities might not be available."
  ON)

option(
  MP_USE_GO_GEOMETRY_WITH_THRUST
  "Set to ON if GraphicsOrigin::geometry uses Cuda/Thrust. If set to false, some functionnalities might not be available."
  ON)
  
option( 
  MP_BUILD_TESTS
  "Set to ON to build the tests."
  ON)
  
#***************
# global flags *
#*******************************************************************************
set( CMAKE_CXX_FLAGS 
  "-g -frounding-math -std=c++14 -fPIC -fopenmp -DGLM_FORCE_SSE2 -DGLM_FORCE_RADIANS -DBOOST_LOG_DYN_LINK -DBOOST_ALL_DYN_LINK"
  CACHE STRING "C++ compiler flags" FORCE )
set( CMAKE_CXX_FLAGS_RELEASE 
  "-O3 -msse2 -UDEBUG -DNDEBUG -DCGAL_NDEBUG -DNO_DEBUG -DEIGEN_NO_DEBUG" 
  CACHE STRING "C++ compiler extra flags for release build type" FORCE )
set( CMAKE_CXX_FLAGS_DEBUG 
  "-O0 -ggdb -Wall -Wextra -DDEBUG" 
  CACHE STRING "C++ compiler extra flags for debug build type" FORCE )
set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lgomp" )


if( ${MP_USE_GO_GEOMETRY_WITH_THRUST} )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGO_USE_CUDA_THRUST" )
endif()
include_directories( "${MP_HEADERS_IN_DIR}" )
  
#*********************
# external libraries *
#*******************************************************************************
set( GRAPHICS_ORIGIN_WITH_CUDA ${MP_USE_GO_GEOMETRY_WITH_CUDA} )
set( GRAPHICS_ORIGIN_WITH_APPLICATION ${MP_USE_GO_APPLICATION} )
set( GRAPHICS_ORIGIN_DIR  $ENV{GRAPHICS_ORIGIN_DIR}
  CACHE FILEPATH "Variable to try to find Graphics-Origin installation path")
find_package( GraphicsOrigin )  

include_directories( ${GRAPHICS_ORIGIN_INCLUDE_DIRS} )
set( MP_INCLUDE_DIRS ${MP_INCLUDE_DIRS} ${GRAPHICS_ORIGIN_INCLUDE_DIRS})
set( MP_LIBRARIES ${MP_LIBRARIES} ${GRAPHICS_ORIGIN_LIBRARIES} )

if( ${MP_USE_CGAL} )
  set( CGAL_CORE_PACKAGE_DIR "/usr"
  CACHE FILEPATH "Variable to try to find CGAL_CORE package i.e. the directory where include/CGAL/CORE can be found" )

  #TODO: I have done quite a mess to compile with the CGAL libraries. 
  #TODO: clean that by using the cmake files given by CGAL or write another find package file for cgal

  find_package( GMP REQUIRED )
  find_package( MPFR REQUIRED )
  find_package( TBB REQUIRED COMPONENTS tbbmalloc )
  	
  if( NOT GMP_FOUND )
    message( fatal "GMP not found")
  endif()
  if( NOT MPFR_FOUND )
    message( fatal "MPFR not found")
  endif()

  include_directories( ${GMP_INCLUDE_DIR} ${MPFR_INCLUDE_DIR} ${TBB_INCLUDE_DIRS})
  set( MP_INCLUDE_DIRS "${MP_INCLUDE_DIRS} ${GMP_INCLUDE_DIR} ${MPFR_INCLUDE_DIR} ${TBB_INCLUDE_DIRS}")
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMP_USE_CGAL -DCGAL_DISABLE_ROUNDING_MATH_CHECK ${TBB_DEFINITIONS}")
  set( MP_LIBRARIES ${MP_LIBRARIES} ${GMP_LIBRARIES} ${MPFR_LIBRARIES} -lCGAL -lCGAL_Core ${TBB_LIBRARIES} ${CGAL_CORE_LIBRARY})
endif()

if( ${MP_BUILD_TESTS} )
  enable_testing()
  set( Boost_USE_MULTITHREADED ON )
  set( Boost_USE_STATIC_LIBS OFF )
  set( Boost_USE_STATIC_RUNTIME OFF )
  find_package( Boost
    REQUIRED
    COMPONENTS unit_test_framework )
  if( Boost_FOUND )
    set( MP_TEST_LIBRARIES 
      ${Boost_LIBRARIES})
	include_directories( "${Boost_INCLUDE_DIRS}" )
	add_subdirectory( ${MP_TEST_IN_DIR} )
  endif()
endif()

configure_file( "${MP_SOURCES_IN_DIR}/median_path.h.in" "${MP_HEADERS_IN_DIR}/median-path/median_path.h")

add_subdirectory( ${MP_SOURCES_IN_DIR} )
add_subdirectory( ${MP_APPLICATIONS_IN_DIR} )
