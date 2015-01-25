set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon
)

set( pimcommon_attachmentdirs_SRCS attachmenttemporaryfilesdirstest.cpp )
kde4_add_unit_test( pimcommon_attachmentdirstest ${pimcommon_attachmentdirs_SRCS})
target_link_libraries( pimcommon_attachmentdirstest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} pimcommon)

