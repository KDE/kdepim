set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon
  ${CMAKE_BINARY_DIR}/pimcommon
)

set( autocorrectiontest_SRCS autocorrectiontest.cpp )
add_executable( autocorrectiontest ${autocorrectiontest_SRCS})
add_test(autocorrectiontest autocorrectiontest)
ecm_mark_as_test(autocorrectiontest)
target_link_libraries( autocorrectiontest Qt5::Test  pimcommon)
