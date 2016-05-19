function( add_mp_app executable_name )
  add_executable( ${executable_name} ${ARGN} )
  target_link_libraries( ${executable_name} median_path ${MP_LIBRARIES} )
  set_property( TARGET ${executable_name} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${MP_APPLICATIONS_OUT_DIR} )
endfunction()
  
add_mp_app( skeletonizer skeletonizer.cc )

add_mp_app( filter_balls_outside_mesh filter_balls_outside_mesh.cc )

add_mp_app( sgp_benchmark sgp_benchmark.cc )

add_mp_app( sgp_benchmark_viewer
	simple_qml_application.cc simple_qml_application.h
	simple_benchmark_viewer_window.cc simple_benchmark_viewer_window.h
	sgp_benchmark_viewer.cc
	simple_camera.cc simple_camera.h
	simple_gl_renderer.h
	skeletons_renderable.cc skeletons_renderable.h)
	
add_mp_app( prepare_mesh_for_benchmark prepare_mesh_for_benchmark.cc )

add_mp_app( export_for_alex export_for_alex.cc )

add_mp_app( regularizer regularizer.cc )

add_custom_command(
	TARGET sgp_benchmark_viewer POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/qml $<TARGET_FILE_DIR:sgp_benchmark_viewer>/qml )