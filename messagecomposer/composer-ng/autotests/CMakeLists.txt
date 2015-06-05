set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set( pimcommon_composerngtest_SRCS richtextcomposertest.cpp )
add_executable( pimcommon_composerngtest ${pimcommon_composerngtest_SRCS})
add_test(pimcommon_composerngtest pimcommon_composerngtest)
ecm_mark_as_test(pimcommon_composerngtest)
target_link_libraries( pimcommon_composerngtest Qt5::Test pimcommon messagecomposer)

