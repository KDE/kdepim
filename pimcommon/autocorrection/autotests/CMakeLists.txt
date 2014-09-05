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


set( lineeditwithautocorrectiontest_SRCS lineeditwithautocorrectiontest.cpp )
add_executable( lineeditwithautocorrectiontest ${lineeditwithautocorrectiontest_SRCS})
add_test(lineeditwithautocorrectiontest lineeditwithautocorrectiontest)
ecm_mark_as_test(lineeditwithautocorrectiontest)
target_link_libraries( lineeditwithautocorrectiontest Qt5::Test  pimcommon)

set( richtexteditwithautocorrectiontest_SRCS richtexteditwithautocorrectiontest.cpp )
add_executable( richtexteditwithautocorrectiontest ${richtexteditwithautocorrectiontest_SRCS})
add_test(richtexteditwithautocorrectiontest richtexteditwithautocorrectiontest)
ecm_mark_as_test(richtexteditwithautocorrectiontest)
target_link_libraries( richtexteditwithautocorrectiontest Qt5::Test  pimcommon)

