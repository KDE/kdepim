set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon
)

set( pimcommon_gravatar_SRCS gravatarresolvurljobtest.cpp )
add_executable( pimcommon_gravatartest ${pimcommon_gravatar_SRCS})
add_test(pimcommon_gravatartest pimcommon_gravatartest)
ecm_mark_as_test(pimcommon_gravatartest)
target_link_libraries( pimcommon_gravatartest Qt5::Test  pimcommon)

set( pimcommon_gravatardownloadpixmapwidget_SRCS gravatardownloadpixmapwidgettest.cpp ../widgets/gravatardownloadpixmapwidget.cpp)
add_executable( pimcommon_gravatardownloadpixmapwidget ${pimcommon_gravatardownloadpixmapwidget_SRCS})
add_test(pimcommon_gravatardownloadpixmapwidget pimcommon_gravatardownloadpixmapwidget)
ecm_mark_as_test(pimcommon_gravatardownloadpixmapwidget)
target_link_libraries( pimcommon_gravatardownloadpixmapwidget Qt5::Test  pimcommon)


