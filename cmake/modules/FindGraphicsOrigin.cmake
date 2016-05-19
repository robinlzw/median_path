# Note: This is the first version of the file. It has been tested on a limited set of plaforms.
#   Proceed with caution and reports any failure/possible improvements. 
#*
# Options
# -------
#   GRAPHICS_ORIGIN_WITH_APPLICATION tells if the Graphisc-Origin application library is needed
#   GRAPHICS_ORIGIN_WITH_OPENCL      tells if OpenCL should be used
#
# Filepath
# --------
#   GRAPHICS_ORIGIN_DIR hint to find the Graphics-Origin installation path, i.e. where the file
#     graphics-origin/graphics_origin.h is.
#
# Variables defined
# -----------------
# GRAPHICS_ORIGIN_FOUND:        true if Graphics-Origin was found
# GRAPHICS_ORIGIN_INCLUDE_DIR:  include directory for Graphics-Origin directories
# GRAPHICS_ORIGIN_LIBRARY_DIR:  Graphics-Origin libraries directory
#
# GRAPHICS_ORIGIN_INCLUDE_DIRS: include directories (include dependencies)
# GRAPHICS_ORIGIN_LIBRARIES:    Graphics-Origin libraries and its dependencies
#
# GRAPHICS_ORIGIN_TOOLS_LIBRARIES: Graphics-Origin tools library and its dependencies
# GRAPHICS_ORIGIN_TOOLS_INCLUDE_DIRS: Graphics-Origin tools and dependencies include directories
#
# GRAPHICS_ORIGIN_GEOMETRY_LIBRARIES: Graphics-Origin geometry library and its dependencies
# GRAPHICS_ORIGIN_GEOMETRY_INCLUDE_DIRS: Graphics-Origin geometry and dependencies include directories
#
# GRAPHICS_ORIGIN_APPLICATION_LIBRARIES: Graphics-Origin application library and its dependencies 
# GRAPHICS_ORIGIN_APPLICATION_INCLUDE_DIRS: Graphics-Origin application and dependencies include directories

find_path( GRAPHICS_ORIGIN_INCLUDE_DIR graphics-origin/graphics_origin.h
  PATHS /usr/local/include /usr/include /opt/local/include ${GRAPHICS_ORIGIN_DIR}/include 
  $ENV{GRAPHICS_ORIGIN_DIR}/include)

if( GRAPHICS_ORIGIN_INCLUDE_DIR )
	set( GRAPHICS_ORIGIN_FOUND ON )  
  set( GRAPHICS_ORIGIN_LIBRARY_DIR "${GRAPHICS_ORIGIN_INCLUDE_DIR}/../lib")
  
  if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
  	set( GRAPHICS_ORIGIN_LIBRARY_DIR "${GRAPHICS_ORIGIN_LIBRARY_DIR}/debug" )
  elseif( CMAKE_BUILD_TYPE STREQUAL "Release" )
    set( GRAPHICS_ORIGIN_LIBRARY_DIR "${GRAPHICS_ORIGIN_LIBRARY_DIR}/release" )
  elseif( CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo" )
    set( GRAPHICS_ORIGIN_LIBRARY_DIR "${GRAPHICS_ORIGIN_LIBRARY_DIR}/relwithdebinfo" )
  else()
    message( SEND_ERROR "${CMAKE_BUILD_TYPE} is not a recognized build type" )
  endif()
  
  set( GRAPHICS_ORIGIN_TOOLS_INCLUDE_DIRS "${GRAPHICS_ORIGIN_INCLUDE_DIR} ${GRAPHICS_ORIGIN_INCLUDE_DIR}/graphics-origin/extlibs" )
	set( GRAPHICS_ORIGIN_GEOMETRY_INCLUDE_DIRS "${GRAPHICS_ORIGIN_INCLUDE_DIR} ${GRAPHICS_ORIGIN_INCLUDE_DIR}/graphics-origin/extlibs" )
	set( GRAPHICS_ORIGIN_APPLICATION_INCLUDE_DIRS "${GRAPHICS_ORIGIN_INCLUDE_DIR} ${GRAPHICS_ORIGIN_INCLUDE_DIR}/graphics-origin/extlibs" )
  
  #################
  # TOOLS LIBRARY #
  ##############################################################################
  find_library( GRAPHICS_ORIGIN_TOOLS_LIBRARIES graphics_origin_tools
  	PATHS
  	  ${GRAPHICS_ORIGIN_LIBRARY_DIR} )

	# boost
	set( Boost_USE_MULTITHREADED ON )
	set( Boost_USE_STATIC_LIBS OFF )
	set( Boost_USE_STATIC_RUNTIME OFF )
	find_package( Boost REQUIRED
	  COMPONENTS 
	    serialization filesystem log log_setup locale random system program_options)
	if( Boost_FOUND )
  	set( GRAPHICS_ORIGIN_TOOLS_LIBRARIES ${GRAPHICS_ORIGIN_TOOLS_LIBRARIES}	${Boost_LIBRARIES})
  	set( GRAPHICS_ORIGIN_TOOLS_INCLUDE_DIRS ${GRAPHICS_ORIGIN_TOOLS_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
  else()
  	message( SEND_ERROR "Could not find all the required Boost libraries" )
  endif()

	# OpenCL
	#   OpenCL_FOUND          - True if OpenCL was found
	#   OpenCL_INCLUDE_DIRS   - include directories for OpenCL
	#   OpenCL_LIBRARIES      - link against this library to use OpenCL
	if( GRAPHICS_ORIGIN_WITH_OPENCL )
	  find_package( OpenCL )
	  if( OpenCL_FOUND )
	    set( GRAPHICS_ORIGIN_TOOLS_LIBRARIES ${GRAPHICS_ORIGIN_TOOLS_LIBRARIES}	${OpenCL_LIBRARIES})
  		set( GRAPHICS_ORIGIN_TOOLS_INCLUDE_DIRS ${GRAPHICS_ORIGIN_TOOLS_INCLUDE_DIRS} ${OpenCL_INCLUDE_DIRS})
	  else()
	    message( SEND_ERROR "Cannot find OpenCL to parallelize code. Disable OpenCL or help cmake to find it." )
	  endif()
	endif()
	
  ####################
  # GEOMETRY LIBRARY #
  ##############################################################################
  find_library( GRAPHICS_ORIGIN_GEOMETRY_LIBRARIES graphics_origin_geometry
  	PATHS ${GRAPHICS_ORIGIN_LIBRARY_DIR} )  
	# openmesh
	find_package( OpenMesh REQUIRED )
	if( OPENMESH_FOUND )
		set( GRAPHICS_ORIGIN_GEOMETRY_LIBRARIES 
  		${GRAPHICS_ORIGIN_GEOMETRY_LIBRARIES}
  		${GRAPHICS_ORIGIN_TOOLS_LIBRARIES}
  		${OPENMESH_LIBRARIES})
  		
  	set( GRAPHICS_ORIGIN_GEOMETRY_INCLUDE_DIRS
  		${GRAPHICS_ORIGIN_TOOLS_INCLUDE_DIRS}
  		${OPENMESH_INCLUDE_DIR})
	else()
	  message( SEND_ERROR "Could not find OpenMesh library for Graphics-Origin::geometry" )	
	endif()
	
  set( GRAPHICS_ORIGIN_LIBRARIES
  	${GRAPHICS_ORIGIN_GEOMETRY_LIBRARIES})
  set( GRAPHICS_ORIGIN_INCLUDE_DIRS
  	${GRAPHICS_ORIGIN_GEOMETRY_INCLUDE_DIRS})

  #######################
  # APPLICATION LIBRARY #
  ##############################################################################
	if( GRAPHICS_ORIGIN_WITH_APPLICATION )
	
  	find_library( GRAPHICS_ORIGIN_APPLICATION_LIBRARIES graphics_origin_application
  		PATHS ${GRAPHICS_ORIGIN_LIBRARY_DIR} )  	
	
	  set( CMAKE_INCLUDE_CURRENT_DIR ON )
	  set( CMAKE_AUTOMOC ON )
	  find_package( Qt5Core REQUIRED )
	  find_package( Qt5Quick REQUIRED )
	  find_package( Qt5Gui REQUIRED )
	  find_package( Qt5Qml REQUIRED )
	  find_package( Qt5Network REQUIRED )
	  find_package( Qt5Widgets REQUIRED )
	 
	  # glew
	  find_package( GLEW REQUIRED )
	  
	  # gl
	  find_package( OpenGL REQUIRED )
	
	set( GRAPHICS_ORIGIN_APPLICATION_LIBRARIES
    	${GRAPHICS_ORIGIN_APPLICATION_LIBRARIES}
    	${GRAPHICS_ORIGIN_GEOMETRY_LIBRARIES}
  		${GLEW_LIBRARIES} ${OPENGL_LIBRARIES}
  		${Qt5Quick_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Qml_LIBRARIES} ${Qt5Network_LIBRARIES} ${Qt5Core_LIBRARIES} ${Qt5Widgets_LIBRARIES})
  	
  	set( GRAPHICS_ORIGIN_APPLICATION_INCLUDE_DIRS
  		${GRAPHICS_ORIGIN_GEOMETRY_INCLUDE_DIRS} 
	  	${GLEW_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR}
  		${Qt5Quick_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Qml_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS})


  	set( GRAPHICS_ORIGIN_LIBRARIES
  		${GRAPHICS_ORIGIN_APPLICATION_LIBRARIES})
  	set( GRAPHICS_ORIGIN_INCLUDE_DIRS
	  	${GRAPHICS_ORIGIN_APPLICATION_INCLUDE_DIRS})  		
	endif()	
else()
	message( SEND_ERROR "Cannot find Graphics-Origin installation path. "
	  "Try to set the variable GRAPHICS_ORIGIN_DIR (currently = ${GRAPHICS_ORIGIN_DIR}" )  	
endif()        
