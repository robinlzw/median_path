add_executable( design_test
	design_test.cc )
	
target_link_libraries( design_test
  median_path ${MP_LIBRARIES})
  
set_property( TARGET design_test
	PROPERTY RUNTIME_OUTPUT_DIRECTORY ${MP_APPLICATIONS_OUT_DIR} )
	
	
add_executable( skeletonizer
	skeletonizer.cc )
	
target_link_libraries( skeletonizer
  median_path ${MP_LIBRARIES})
  
set_property( TARGET skeletonizer
	PROPERTY RUNTIME_OUTPUT_DIRECTORY ${MP_APPLICATIONS_OUT_DIR} )
	
	
add_executable( filter_balls_outside_mesh
	filter_balls_outside_mesh.cc )
	
target_link_libraries( filter_balls_outside_mesh
  median_path ${MP_LIBRARIES})
  
set_property( TARGET filter_balls_outside_mesh
	PROPERTY RUNTIME_OUTPUT_DIRECTORY ${MP_APPLICATIONS_OUT_DIR} )