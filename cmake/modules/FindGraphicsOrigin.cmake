# Note: This is the first version of the file. It has been tested on a limited set of plaforms.
#   Proceed with caution and reports any failure/possible improvements. 
#
# This module reads the following variables:
#
# GRAPHICS_ORIGIN_DIR: variable to specify Graphics-Origin installation path
# GRAPHICS_ORIGIN_WITH_CUDA: tells if CUDA is used in graphics origin libraries
# GRAPHICS_ORIGIN_WITH_APPLICATION: tells if the graphics origin application library is needed
#
# try to find Graphics-Origin and set the following variables:
# GRAPHICS_ORIGIN_FOUND:   if the library was found
# GRAPHICS_ORIGIN_INCLUDE_DIR: include directory for Graphics-Origin directories
# GRAPHICS_ORIGIN_INCLUDE_DIRS: include directories (include dependencies)
# GRAPHICS_ORIGIN_LIBRARIES:    Graphics-Origin libraries and its dependencies
# GRAPHICS_ORIGIN_LIBRARY_DIR:  Graphics-Origin libraries directory
#
# GRAPHICS_ORIGIN_TOOLS_LIBRARIES: Graphics-Origin tools library and its dependencies
# GRAPHICS_ORIGIN_TOOLS_INCLUDE_DIRS: Graphics-Origin tools and dependencies include directories
# GRAPHICS_ORIGIN_GEOMETRY_LIBRARIES: Graphics-Origin geometry library and its dependencies
# GRAPHICS_ORIGIN_GEOMETRY_INCLUDE_DIRS: Graphics-Origin geometry and dependencies include directories
# GRAPHICS_ORIGIN_APPLICATION_LIBRARIES: Graphics-Origin application library and its dependencies 
# GRAPHICS_ORIGIN_APPLICATION_INCLUDE_DIRS: Graphics-Origin application and dependencies include directories

find_path( GRAPHICS_ORIGIN_INCLUDE_DIR graphics-origin/graphics_origin.h
  PATHS /usr/local/include /usr/include /opt/local/include ${GRAPHICS_ORIGIN_DIR}/include 
  $ENV{GRAPHICS_ORIGIN_DIR}/include)

if( GRAPHICS_ORIGIN_INCLUDE_DIR )
	set( GRAPHICS_ORIGIN_FOUND ON )  
  set( GRAPHICS_ORIGIN_LIBRARY_DIR "${GRAPHICS_ORIGIN_INCLUDE_DIR}/../lib")
  
  if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
  	set( GRAPHICS_ORIGIN_LIBRARY_DIR ${GRAPHICS_ORIGIN_LIBRARY_DIR}/debug )
  else()
    set( GRAPHICS_ORIGIN_LIBRARY_DIR ${GRAPHICS_ORIGIN_LIBRARY_DIR}/release )
  endif()

  #################
  # TOOLS LIBRARY #
  ##############################################################################
  find_library( GRAPHICS_ORIGIN_TOOLS_LIBRARIES graphics_origin_tools
  	PATHS ${GRAPHICS_ORIGIN_LIBRARY_DIR} )

	# boost
	set( Boost_USE_MULTITHREADED ON )
	set( Boost_USE_STATIC_LIBS OFF )
	set( Boost_USE_STATIC_RUNTIME OFF )
	find_package( Boost
	  REQUIRED
	  COMPONENTS serialization filesystem log log_setup locale random system )
	
	if( Boost_FOUND )

  	set( GRAPHICS_ORIGIN_TOOLS_LIBRARIES
  	  ${GRAPHICS_ORIGIN_TOOLS_LIBRARIES} 
  		${Boost_LIBRARIES})
  	
  	set( GRAPHICS_ORIGIN_TOOLS_INCLUDE_DIRS
	    ${GRAPHICS_ORIGIN_INCLUDE_DIR}
    	${Boost_INCLUDE_DIRS})
  else()
  	message( SEND_ERROR "Could not find all the required Boost libraries" )
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
	# cuda
	if( GRAPHICS_ORIGIN_WITH_CUDA )
	  find_package( CUDA REQUIRED )
	  if( NOT CUDA_FOUND )
	    message( SEND_ERROR "Could not find CUDA libraries for Graphics-Origin::geometry" )
	  else()
	    set( GRAPHICS_ORIGIN_GEOMETRY_LIBRARY ${GRAPHICS_ORIGIN_GEOMETRY_LIBRARY}
	    	${CUDA_CUDA_LIBRARY} ${CUDA_CUDART_LIBRARY})
	    set( GRAPHICS_ORIGIN_GEOMETRY_INCLUDE_DIRS ${GRAPHICS_ORIGIN_GEOMETRY_INCLUDE_DIRS}
	    	${CUDA_TOOLKIT_INCLUDE} )
	    	
	  endif()
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
	 
	  # glew
	  find_package( GLEW REQUIRED )
	  
	  # gl
	  find_package( OpenGL REQUIRED )
	
	  set( GRAPHICS_ORIGIN_APPLICATION_LIBRARIES
    	${GRAPHICS_ORIGIN_APPLICATION_LIBRARIES}
    	${GRAPHICS_ORIGIN_GEOMETRY_LIBRARIES}
  		${GLEW_LIBRARIES} ${OPENGL_LIBRARIES}
  		${Qt5Quick_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Qml_LIBRARIES} ${Qt5Network_LIBRARIES} ${Qt5Core_LIBRARIES})
  	
  	set( GRAPHICS_ORIGIN_APPLICATION_INCLUDE_DIRS
  		${GRAPHICS_ORIGIN_GEOMETRY_INCLUDE_DIRS} 
	  	${GLEW_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR}
  		${Qt5Quick_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Qml_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS})

  	set( GRAPHICS_ORIGIN_LIBRARIES
  		${GRAPHICS_ORIGIN_APPLICATION_LIBRARIES})
  	set( GRAPHICS_ORIGIN_INCLUDE_DIRS
	  	${GRAPHICS_ORIGIN_APPLICATION_INCLUDE_DIRS})  		
	endif()	
else()
	message( SEND_ERROR "Cannot find Graphics-Origin installation path. "
	  "Try to set the variable GRAPHICS_ORIGIN_DIR (currently = ${GRAPHICS_ORIGIN_DIR}" )  	
endif()        