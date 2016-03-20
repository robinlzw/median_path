file(
  GLOB_RECURSE
  HEADER_FILES
  "${MP_TEST_IN_DIR}/*.h")
  
file(
  GLOB_RECURSE
  SOURCE_FILES
  "${MP_TEST_IN_DIR}/*.cc")
  
add_executable( fulltest
  ${HEADER_FILES}
  ${SOURCE_FILES})
  
target_link_libraries( fulltest
  ${GRAPHICS_ORIGIN_LIBRARIES}
  ${MP_LIBRARIES}
  median_path
  ${MP_TEST_LIBRARIES} )
  
  
add_test(NAME fulltest
  WORKING_DIRECTORY "${MP_TEST_OUT_DIR}"
  COMMAND fulltest ${MP_TEST_COMMAND_ARGUMENTS})
   
