# NAME name of the graphics tutorial
#   There should be a NAME.cc file and a NAME/ directory in the current source directory.
#   All common sources (if this is a graphic application), sources in NAME/ and NAME.cc
#   are automatically taken into account.
#   QML and Shader files in the NAME/ directory will be copied to the binary directory.
#
# GRAPHIC option to pass to define a graphical application
#
# SHADERS list of shaders outside of the NAME/ directory required to launch the tutorial
#
# MESHES list of mesh files required for this tutorial
#
# LIBRARIES list of libraries to link the tutorial with
function( mp_add_application )
  set( options GRAPHIC )
  set( oneValueArgs NAME )
  set( multiValueArgs SHADERS MESHES LIBRARIES )
  cmake_parse_arguments( APP "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
  
  if( APP_GRAPHIC )
    if( NOT ${MP_BUILD_GRAPHIC_APPS} )
      return()
    endif()
    file( GLOB_RECURSE APP_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/${APP_NAME}/*.cc
      ${CMAKE_CURRENT_SOURCE_DIR}/${APP_NAME}/*.h
      ${CMAKE_CURRENT_SOURCE_DIR}/common/*.cc
      ${CMAKE_CURRENT_SOURCE_DIR}/common/*.h
      ${APP_NAME}.cc
    )
    file( GLOB_RECURSE APP_QML
      ${CMAKE_CURRENT_SOURCE_DIR}/${APP_NAME}/*.qml
      ${MP_QML_DIR}/*.qml
      ${MP_QML_DIR}/qmldir
    )
    file( GLOB_RECURSE APP_SHADERS
      ${CMAKE_CURRENT_SOURCE_DIR}/${APP_NAME}/*.vert
      ${CMAKE_CURRENT_SOURCE_DIR}/${APP_NAME}/*.tes
      ${CMAKE_CURRENT_SOURCE_DIR}/${APP_NAME}/*.tcs
      ${CMAKE_CURRENT_SOURCE_DIR}/${APP_NAME}/*.geom
      ${CMAKE_CURRENT_SOURCE_DIR}/${APP_NAME}/*.frag
      ${APP_SHADERS}
    )
   
    mp_add_executable( 
      NAME ${APP_NAME}
      SOURCES ${APP_SOURCES}
      LIBRARIES ${APP_LIBRARIES}
    )

    if( NOT "${APP_QML}" STREQUAL "" )
      set( _destination $<TARGET_FILE_DIR:${APP_NAME}>/qml )
      add_custom_command( TARGET ${APP_NAME}
        DEPENDS ${APP_QML}
        COMMENT "Copying QML files for application ${APP_NAME}"
        COMMAND ${CMAKE_COMMAND} -E make_directory ${_destination}
        COMMAND ${CMAKE_COMMAND} -E copy ${APP_QML} ${_destination}
      )
    endif()

    if( NOT "${APP_SHADERS}" STREQUAL "" )
      set( _destination $<TARGET_FILE_DIR:${APP_NAME}>/shaders )
      add_custom_command( TARGET ${APP_NAME}
        DEPENDS ${APP_SHADERS}
        COMMENT "Copying shader files for application ${APP_NAME}"
        COMMAND ${CMAKE_COMMAND} -E make_directory ${_destination}
        COMMAND ${CMAKE_COMMAND} -E copy ${APP_SHADERS} ${_destination}
      )
    endif()
    
    if( NOT "${APP_MESHES}" STREQUAL "" )
      set( _destination $<TARGET_FILE_DIR:${APP_NAME}>/meshes )
      add_custom_command( TARGET ${APP_NAME}
        DEPENDS ${APP_MESHES}
        COMMENT "Copying mesh files for application ${APP_NAME}"
        COMMAND ${CMAKE_COMMAND} -E make_directory ${_destination}
        COMMAND ${CMAKE_COMMAND} -E copy ${APP_MESHES} ${_destination}
      )
    endif()    
    
  else()
    file( GLOB_RECURSE APP_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/${APP_NAME}/*.cc
      ${CMAKE_CURRENT_SOURCE_DIR}/${APP_NAME}/*.h
      ${APP_NAME}.cc
    )
    mp_add_executable( 
      NAME ${APP_NAME}
      SOURCES ${APP_SOURCES}
      LIBRARIES ${APP_LIBRARIES}
    )
  endif()

 
  
endfunction( mp_add_application )

mp_add_application(
  NAME skeletonizer
  LIBRARIES ${MP_LIBRARIES} median_path
)

mp_add_application(
  NAME filter_balls_outside_mesh
  LIBRARIES ${MP_LIBRARIES} median_path
)

mp_add_application(
  NAME regularizer
  LIBRARIES ${MP_LIBRARIES} median_path
)

#mp_add_application(
#  NAME new_skeletonizer
#  LIBRARIES ${MP_LIBRARIES} median_path)


mp_add_application(
  NAME benchmark_viewer
  GRAPHIC
  LIBRARIES ${MP_LIBRARIES} median_path)

#add_mp_app( prepare_mesh_for_benchmark prepare_mesh_for_benchmark.cc )


