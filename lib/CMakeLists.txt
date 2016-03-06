file(
  GLOB_RECURSE
  HEADER_FILES
  "${MP_HEADERS_IN_DIR}/median-path/*.h"
  "${MP_HEADERS_IN_DIR}/median-path/detail/*.h")
  
file(
  GLOB_RECURSE
  SOURCE_FILES
  "${MP_SOURCES_IN_DIR}/*.cc")
  
add_library(
  median_path SHARED
  ${SOURCE_FILES} ${HEADER_FILES})
  
install( TARGETS median_path
  CONFIGURATIONS Debug
  DESTINATION lib/debug )
  
install( TARGETS median_path
  CONFIGURATIONS Release
  DESTINATION lib/release )  