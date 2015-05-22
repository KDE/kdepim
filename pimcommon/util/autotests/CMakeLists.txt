include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon
)

set(pimcommon_util_editorutiltest_SRCS editorutiltest.cpp ../editorutil.cpp)
add_executable( editorutiltest ${pimcommon_util_editorutiltest_SRCS} )
add_test(editorutiltest editorutiltest)
ecm_mark_as_test(editorutiltest)
target_link_libraries( editorutiltest
   Qt5::Test
   Qt5::Gui
   Qt5::Core
   pimcommon
)


set(pimcommon_util_networkutil_SRCS networkutiltest.cpp )
add_executable( networkutiltest ${pimcommon_util_networkutil_SRCS} )
add_test(networkutiltest networkutiltest)
ecm_mark_as_test(networkutiltest)
target_link_libraries( networkutiltest
   Qt5::Test
   Qt5::Core
   pimcommon
)

