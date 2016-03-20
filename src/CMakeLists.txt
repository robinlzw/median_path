add_executable( design_test
	design_test.cc )
	
target_link_libraries( design_test
  ${GRAPHICS_ORIGIN_GEOMETRY_LIBRARIES})
  
set_property( TARGET design_test
	PROPERTY RUNTIME_OUTPUT_DIRECTORY ${MP_APPLICATIONS_OUT_DIR} )
	
	
add_executable( skeletonizer
	skeletonizer.cc )
	
message( "LINKING skeletonizer to ${MP_LIBRARIES}")	
target_link_libraries( skeletonizer
  median_path ${MP_LIBRARIES})
  
set_property( TARGET skeletonizer
	PROPERTY RUNTIME_OUTPUT_DIRECTORY ${MP_APPLICATIONS_OUT_DIR} )