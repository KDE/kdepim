set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set( pimcommon_attachmentdirs_SRCS attachmenttemporaryfilesdirstest.cpp )
add_executable( pimcommon_attachmentdirstest ${pimcommon_attachmentdirs_SRCS})
add_test(pimcommon_attachmentdirstest pimcommon_attachmentdirstest)
ecm_mark_as_test(pimcommon_attachmentdirstest)
target_link_libraries( pimcommon_attachmentdirstest Qt5::Test )


