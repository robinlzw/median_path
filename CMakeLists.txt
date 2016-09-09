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
# Finding external libraries
# --------------------------
#
# Here are some variables to tune in order to find all libraries related to Graphics Origin
#   BOOST_ROOT            where boost has been installed
#   BOOST_INCLUDEDIR      Preferred include directory e.g. <prefix>/include
#   BOOST_LIBRARYDIR      Preferred library directory e.g. <prefix>/lib
#   Boost_NO_SYSTEM_PATHS Set to ON to disable searching in locations not specified by these hint variables. Default is OFF.
#   OPENMESH_DIR          where to find OpenMesh/Core/Mesh/PolyMeshT.hh
#   LIBNOISE_DIR          where to find either noise/noise.h or include/noise/noise.h
#   GLEW_DIR              where to find GL/glew.h
#   FREEIMAGE_DIR         where to find FreeImage.h
#
# In case cmake cannot find the FindGraphicsOrigin.cmake, set the variable
# GRAPHICS_ORIGIN_MODULE_PATH to its directory.
#
# Compilation Options
# -------------------
# MP_SKELETON_NO_CHECK Add this definition to perform less checks while accessing to skeleton elements.
#
# Build types
# -----------
# Valid values of CMAKE_BUILD_TYPE:
# - Release
# - RelWithDebInfo
# - Debug
 

#**********************
# cmake configuration *
#*******************************************************************************
cmake_minimum_required( VERSION 3.1.0 )
list( APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" )
list( APPEND CMAKE_MODULE_PATH "${GRAPHICS_ORIGIN_MODULE_PATH}" )

# Graphics-Origin project
project( Median-Path )
set( Median-Path_VERSION_MAJOR 1 )
set( Median-Path_VERSION_MINOR 0 )

# For linux builds, Release is the default build type
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
set( MP_APP_OUT_DIR_RELEASE "${MP_OUT_DIR}/release/bin" )
set( MP_TUTORIAL_OUT_DIR_RELEASE "${MP_OUT_DIR}/release/tutorial" )
set( MP_TEST_OUT_DIR_RELEASE "${MP_OUT_DIR}/release/tests" )

set( MP_LIBRARY_OUT_DIR_RELWITHDEBINFO "${MP_OUT_DIR}/relwithdebinfo/lib" )
set( MP_APP_OUT_DIR_RELWITHDEBINFO "${MP_OUT_DIR}/relwithdebinfo/bin" )
set( MP_TUTORIAL_OUT_DIR_RELWITHDEBINFO "${MP_OUT_DIR}/relwithdebinfo/tutorial" )
set( MP_TEST_OUT_DIR_RELWITHDEBINFO "${MP_OUT_DIR}/relwithdebinfo/tests" )

#******************
# project options *
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
  
option(MP_CGAL_EPECK_BUG
  "Set to ON to hack the bug in CGAL Epeck Kernel. The geometry will be less precise with Polar and Voronoi balls, but will not cause segmentation fault"
  ON)
  
if( MP_BUILD_GRAPHICS_APPS AND NOT MP_BUILD_APPS )
  message( SEND_ERROR "Cannot build graphic applications if applications are not build.")
  message( SEND_ERROR "You should either set MP_BUILD_GRAPHICS_APPS to OFF or MP_BUILD_APPS to ON")
endif()
  
#*********************
# external libraries *
#*******************************************************************************
include_directories( "${MP_HEADERS_IN_DIR}" )

# find Graphics-Origin
set( GRAPHICS_ORIGIN_WITH_APPLICATION ${MP_BUILD_GRAPHIC_APPS} )
set( GRAPHICS_ORIGIN_DIR  $ENV{GRAPHICS_ORIGIN_DIR}
  CACHE FILEPATH "Variable to try to find Graphics-Origin installation path")
find_package( GraphicsOrigin REQUIRED )  
include_directories( ${GRAPHICS_ORIGIN_INCLUDE_DIRS} )
set( MP_INCLUDE_DIRS ${MP_INCLUDE_DIRS} ${GRAPHICS_ORIGIN_INCLUDE_DIRS})
set( MP_LIBRARIES ${MP_LIBRARIES} ${GRAPHICS_ORIGIN_LIBRARIES} )

# find CGAL
find_package( TBB REQUIRED QUIET COMPONENTS tbbmalloc )
if( NOT TBB_FOUND )
  message( SEND_ERROR "TBB not found")
endif()

find_package(CGAL QUIET COMPONENTS Core )
if( NOT CGAL_FOUND )
  message( SEND_ERROR "CGAL not found" )
endif()

include( ${CGAL_USE_FILE} )
include_directories( ${TBB_INCLUDE_DIRS} )
set( MP_INCLUDE_DIRS ${MP_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS})
set( MP_LIBRARIES ${MP_LIBRARIES} ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES} ${TBB_LIBRARIES}) 

#***************
# global flags *
#*******************************************************************************
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
  set( CMAKE_CXX_FLAGS 
    "${GRAPHICS_ORIGIN_CXX_FLAGS} -frounding-math -DGLM_FORCE_SSE2 -DGLM_FORCE_RADIANS -DBOOST_LOG_DYN_LINK -DBOOST_ALL_DYN_LINK"
    CACHE STRING "GNU C++ compiler flags" FORCE )
  set( CMAKE_CXX_FLAGS_RELEASE 
    "-O3 -msse2 -UDEBUG -DNDEBUG -DNO_DEBUG -DCGAL_NDEBUG" 
    CACHE STRING "GNU C++ compiler extra flags for release build type" FORCE )
  set( CMAKE_CXX_FLAGS_DEBUG 
    "-O0 -ggdb -Wall -Wextra -DDEBUG" 
    CACHE STRING "GNU C++ compiler extra flags for debug build type" FORCE )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_OMP -DTHRUST_HOST_SYSTEM=THRUST_HOST_SYSTEM_OMP" )
  if( MP_CGAL_EPECK_BUG )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMP_CGAL_MUST_USE_EPICK" )
  endif()
  set( CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${GRAPHICS_ORIGIN_LINKER_FLAGS}" )
elseif( CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
  set( CMAKE_CXX_FLAGS
    "${GRAPHICS_ORIGIN_CXX_FLAGS} /EHsc /D_USE_MATH_DEFINES /DGLM_FORCE_SSE2 /DGLM_FORCE_RADIANS /DBOOST_LOG_DYN_LINK /DBOOST_ALL_DYN_LINK"
	CACHE STRING "MSVC C++ compiler flags" FORCE )
  set( CMAKE_CXX_FLAGS_RELEASE
    "/Ox /UDEBUG /DNDEBUG /DNO_DEBUG"
	CACHE STRING "MSVC C++ compiler extrat flags for release build type" FORCE )
  set( CMAKE_CXX_FLAGS_DEBUG 
    "/Od /Zi /DDEBUG" 
    CACHE STRING "MSVC C++ compiler extra flags for debug build type" FORCE )	
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_OMP /DTHRUST_HOST_SYSTEM=THRUST_HOST_SYSTEM_OMP" )
  if( MP_CGAL_EPECK_BUG )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DMP_CGAL_MUST_USE_EPICK" )
  endif()
  set( CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${GRAPHICS_ORIGIN_LINKER_FLAGS}" )
else()
  message( SEND_ERROR "Compiler ${CMAKE_CXX_COMPILER_ID} not managed")
endif()

#***********
# Products *
#*******************************************************************************
# median_path.h from a configuration file
configure_file( "${MP_SOURCES_IN_DIR}/median_path.h.in" "${MP_HEADERS_IN_DIR}/median-path/median_path.h")

function( mp_add_lib )
  set( options "" )
  set( oneValueArgs NAME )
  set( multiValueArgs SOURCES LIBRARIES )
  cmake_parse_arguments( LIB "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
  
  add_library( ${LIB_NAME} SHARED ${LIB_SOURCES} )
  
  set_target_properties( ${LIB_NAME} 
	PROPERTIES
	COMPILE_DEFINITIONS "MP_EXPORTS"
	
	IMPORTED_LOCATION_RELEASE "${MP_LIBRARY_OUT_DIR_RELEASE}/${LIB_NAME}"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${MP_LIBRARY_OUT_DIR_RELEASE}"
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${MP_LIBRARY_OUT_DIR_RELEASE}"
	LIBRARY_OUTPUT_DIRECTORY_RELEASE "${MP_LIBRARY_OUT_DIR_RELEASE}"
    
	IMPORTED_LOCATION_DEBUG "${MP_LIBRARY_OUT_DIR_DEBUG}/${LIB_NAME}"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${MP_LIBRARY_OUT_DIR_DEBUG}"
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${MP_LIBRARY_OUT_DIR_DEBUG}"
	LIBRARY_OUTPUT_DIRECTORY_DEBUG "${MP_LIBRARY_OUT_DIR_DEBUG}"
	
    IMPORTED_LOCATION_RELWITHDEBINFO "${MP_LIBRARY_OUT_DIR_RELWITHDEBINFO}/${LIB_NAME}"
	RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${MP_LIBRARY_OUT_DIR_RELWITHDEBINFO}"
	ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${MP_LIBRARY_OUT_DIR_RELWITHDEBINFO}"
	LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${MP_LIBRARY_OUT_DIR_RELWITHDEBINFO}"
  )
  
  target_link_libraries( ${LIB_NAME} ${LIB_LIBRARIES} )
  
endfunction( mp_add_lib )

# build median path library
add_subdirectory( ${MP_SOURCES_IN_DIR} )

# build tests
if( ${MP_BUILD_TESTS} )
  enable_testing()
  set( Boost_USE_MULTITHREADED ON )
  set( Boost_USE_STATIC_LIBS OFF )
  set( Boost_USE_STATIC_RUNTIME OFF )
  find_package( Boost
    REQUIRED QUIET
    COMPONENTS unit_test_framework )
  if( Boost_FOUND )
    set( MP_TEST_LIBRARIES 
      ${Boost_LIBRARIES})
	include_directories( "${Boost_INCLUDE_DIRS}" )
	add_subdirectory( ${MP_TEST_IN_DIR} )
  endif()
endif()

# build apps
if( ${MP_BUILD_APPS} )
  function( mp_add_executable )
    set( options "" )
    set( oneValueArgs NAME )
    set( multiValueArgs SOURCES LIBRARIES )
    cmake_parse_arguments( EXE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
    
    add_executable( ${EXE_NAME} ${EXE_SOURCES} )
    
    target_link_libraries( ${EXE_NAME} ${EXE_LIBRARIES} )
    
    set_target_properties( ${EXE_NAME}
	  PROPERTIES                       
	  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${MP_APP_OUT_DIR_RELEASE}"
	  ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${MP_APP_OUT_DIR_RELEASE}"
	  LIBRARY_OUTPUT_DIRECTORY_RELEASE "${MP_APP_OUT_DIR_RELEASE}"

	  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${MP_APP_OUT_DIR_DEBUG}"
	  ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${MP_APP_OUT_DIR_DEBUG}"
	  LIBRARY_OUTPUT_DIRECTORY_DEBUG "${MP_APP_OUT_DIR_DEBUG}"
  
	  RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${MP_APP_OUT_DIR_RELWITHDEBINFO}"
	  ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${MP_APP_OUT_DIR_RELWITHDEBINFO}"
	  LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${MP_APP_OUT_DIR_RELWITHDEBINFO}"
	)
  endfunction( mp_add_executable )
  
  add_subdirectory( ${MP_APP_IN_DIR} )
  
endif()

# build documentation
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
"MP_BUILD_DOC          = ${MP_BUILD_DOC}\n"
"MP_CGAL_EPECK_BUG     = ${MP_CGAL_EPECK_BUG}")
