add_executable( design_test
	design_test.cc )
	
target_link_libraries( design_test
  ${GRAPHICS_ORIGIN_GEOMETRY_LIBRARIES})
  
set_property( TARGET design_test
	PROPERTY RUNTIME_OUTPUT_DIRECTORY ${MP_APPLICATIONS_OUT_DIR} )
	
	
add_executable( skeletonizer
	skeletonizer.cc )
	
target_link_libraries( skeletonizer
  ${GRAPHICS_ORIGIN_GEOMETRY_LIBRARIES} median_path)
  
set_property( TARGET skeletonizer
	PROPERTY RUNTIME_OUTPUT_DIRECTORY ${MP_APPLICATIONS_OUT_DIR} )