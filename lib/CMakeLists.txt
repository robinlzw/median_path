file(GLOB_RECURSE MEDIAN_PATH_SOURCES
  "${MP_HEADERS_IN_DIR}/median-path/*.h"
  "${MP_HEADERS_IN_DIR}/median-path/detail/*.h"
  "${MP_SOURCES_IN_DIR}/*.cc"
)

mp_add_lib( NAME median_path
  SOURCES ${MEDIAN_PATH_SOURCES}
  LIBRARIES ${MP_LIBRARIES}
)
