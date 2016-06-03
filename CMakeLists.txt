#*******************************************************************************
# MEDIAN PATH BUILDING SYSTEM                                                  *
#*******************************************************************************
# Options
# -------
# MP_BUILD_DOC          Set to ON to build the documentation
# MP_BUILD_TEST         Set to ON to build the (currently partial) set of tests
# MP_BUILD_APPS         Set to ON to build Median-Path applications
# MP_BUILD_GRAPHIC_APPS Set to ON to build also graphic applications when MP_BUILD_APPS is ON. 
#
# Compilation Options
# -------------------
# MP_SKELETON_NO_CHECK Add this definition to perform less checks while accessing to skeleton elements.

#**********************
# cmake configuration *
#*******************************************************************************
cmake_minimum_required( VERSION 3.1.0 )
set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# Graphics-Origin project
project( Median-Path )
set( Median-Path_VERSION_MAJOR 1 )
set( Median-Path_VERSION_MINOR 0 )

if( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
  if( NOT CMAKE_BUILD_TYPE )
    set( CMAKE_BUILD_TYPE "Release" 
      CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo." FORCE)
  endif(NOT CMAKE_BUILD_TYPE)
endif()

#****************************
# directories configuration *
#*******************************************************************************
set( MP_HEADERS_IN_DIR "${CMAKE_SOURCE_DIR}" )
set( MP_SOURCES_IN_DIR "${CMAKE_SOURCE_DIR}/lib" )
set( MP_TUTORIAL_IN_DIR "${CMAKE_SOURCE_DIR}/tutorial" )
set( MP_APP_IN_DIR "${CMAKE_SOURCE_DIR}/src" )
set( MP_TEST_IN_DIR "${CMAKE_SOURCE_DIR}/tests" )
set( MP_SHADERS_DIR "${CMAKE_SOURCE_DIR}/shaders" )
set( MP_MESHES_DIR "${CMAKE_SOURCE_DIR}/meshes" )
set( MP_QML_DIR "${CMAKE_SOURCE_DIR}/qml" )
set( MP_OUT_DIR "${CMAKE_BINARY_DIR}")

set( MP_LIBRARY_OUT_DIR_DEBUG "${MP_OUT_DIR}/debug/lib" )
set( MP_APP_OUT_DIR_DEBUG "${MP_OUT_DIR}/debug/bin" )
set( MP_TUTORIAL_OUT_DIR_DEBUG "${MP_OUT_DIR}/debug/tutorial" )
set( MP_TEST_OUT_DIR_DEBUG "${MP_OUT_DIR}/debug/tests" )

set( MP_LIBRARY_OUT_DIR_RELEASE "${MP_OUT_DIR}/release/lib" )
set( MP_APPLICATION_OUT_DIR_RELEASE "${MP_OUT_DIR}/release/bin" )
set( MP_TUTORIAL_OUT_DIR_RELEASE "${MP_OUT_DIR}/release/tutorial" )
set( MP_TEST_OUT_DIR_RELEASE "${MP_OUT_DIR}/release/tests" )

set( MP_LIBRARY_OUT_DIR_RELWITHDEBINFO "${MP_OUT_DIR}/relwithdebinfo/lib" )
set( MP_APPLICATION_OUT_DIR_RELWITHDEBINFO "${MP_OUT_DIR}/relwithdebinfo/bin" )
set( MP_TUTORIAL_OUT_DIR_RELWITHDEBINFO "${MP_OUT_DIR}/relwithdebinfo/tutorial" )
set( MP_TEST_OUT_DIR_RELWITHDEBINFO "${MP_OUT_DIR}/relwithdebinfo/tests" )

#*********************
# projection options *
#*******************************************************************************
option( MP_BUILD_APPS
  "Set to ON to build applications."
  ON)

option( MP_BUILD_GRAPHIC_APPS
  "Set to ON to build graphic applications when MP_BUILD_APPS is ON too. This require to have built Graphic-Origin Application library."
  ON)

option(MP_BUILD_TESTS 
  "Set to ON to build the tests." 
  ON)

option(MP_BUILD_DOC 
  "Set to ON to build the documentation." 
  ON)
  
if( MP_BUILD_GRAPHICS_APPS AND NOT MP_BUILD_APPS )
  message( SEND_ERROR "Cannot build graphic applications if applications are not build.")
  message( SEND_ERROR "You should either set MP_BUILD_GRAPHICS_APPS to OFF or MP_BUILD_APPS to ON")
endif()
  
#*********************
# external libraries *
#*******************************************************************************
include_directories( "${MP_HEADERS_IN_DIR}" )

set( GRAPHICS_ORIGIN_WITH_APPLICATION ${MP_BUILD_GRAPHIC_APPS} )
set( GRAPHICS_ORIGIN_DIR  $ENV{GRAPHICS_ORIGIN_DIR}
  CACHE FILEPATH "Variable to try to find Graphics-Origin installation path")
find_package( GraphicsOrigin )  
include_directories( ${GRAPHICS_ORIGIN_INCLUDE_DIRS} )
set( MP_INCLUDE_DIRS ${MP_INCLUDE_DIRS} ${GRAPHICS_ORIGIN_INCLUDE_DIRS})
set( MP_LIBRARIES ${MP_LIBRARIES} ${GRAPHICS_ORIGIN_LIBRARIES} )


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
set( MP_LIBRARIES ${MP_LIBRARIES} ${GMP_LIBRARIES} ${MPFR_LIBRARIES} -lCGAL -lCGAL_Core ${TBB_LIBRARIES} ${CGAL_CORE_LIBRARY})

#***************
# global flags *
#*******************************************************************************
if( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
  set( CMAKE_CXX_FLAGS 
    "${CMAKE_CXX_FLAGS} ${GRAPHICS_ORIGIN_CXX_FLAGS} -frounding-math -std=c++14 -fPIC -DGLM_FORCE_SSE2 -DGLM_FORCE_RADIANS -DBOOST_LOG_DYN_LINK -DBOOST_ALL_DYN_LINK"
    CACHE STRING "GNU C++ compiler flags" FORCE )
  set( CMAKE_CXX_FLAGS_RELEASE 
    "${CMAKE_CXX_FLAGS_RELEASE} -O3 -msse2 -UDEBUG -DNDEBUG -DNO_DEBUG -DCGAL_NDEBUG" 
    CACHE STRING "GNU C++ compiler extra flags for release build type" FORCE )
  set( CMAKE_CXX_FLAGS_DEBUG 
    "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb -Wall -Wextra -DDEBUG" 
    CACHE STRING "GNU C++ compiler extra flags for debug build type" FORCE )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_OMP" )
  set( CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${GRAPHICS_ORIGIN_LINKER_FLAGS}" )
elseif( CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
  set( CMAKE_CXX_FLAGS
    "${GRAPHICS_ORIGIN_CXX_FLAGS} /EHsc /openmp /D_USE_MATH_DEFINES /DGLM_FORCE_SSE2 /DGLM_FORCE_RADIANS /DBOOST_LOG_DYN_LINK /DBOOST_ALL_DYN_LINK"
	CACHE STRING "MSVC C++ compiler flags" FORCE )
  set( CMAKE_CXX_FLAGS_RELEASE
    "${CMAKE_CXX_FLAGS_RELEASE} /Ox /UDEBUG /DNDEBUG /DNO_DEBUG"
	CACHE STRING "MSVC C++ compiler extrat flags for release build type" FORCE )
  set( CMAKE_CXX_FLAGS_DEBUG 
    "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi /DDEBUG" 
    CACHE STRING "MSVC C++ compiler extra flags for debug build type" FORCE )	
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_OMP" )
  set( CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${GRAPHICS_ORIGIN_LINKER_FLAGS}" )
else()
  message( SEND_ERROR "Compiler ${CMAKE_CXX_COMPILER_ID} not managed")
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
add_subdirectory( ${MP_APP_IN_DIR} )

if( ${MP_BUILD_DOC} )
  find_package( Doxygen )
  if( DOXYGEN_FOUND )
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        file(MAKE_DIRECTORY doc) 
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doc
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
  else()
    message( ERROR "Cannot find Doxygen to build the documentation" )
  endif()
endif()

message(
"MP_BUILD_APPS         = ${MP_BUILD_APPS}\n"
"MP_BUILD_GRAPHIC_APPS = ${MP_BUILD_GRAPHIC_APPS}\n"
"MP_BUILD_TESTS        = ${MP_BUILD_TESTS}\n"
"MP_BUILD_DOC          = ${MP_BUILD_DOC}")
