set( mailcommon_searchpatterntest_source searchpatterntest.cpp )
add_executable( searchpatterntest ${mailcommon_searchpatterntest_source})
add_test(searchpatterntest searchpatterntest)
ecm_mark_as_test(searchpatterntest)
target_link_libraries( searchpatterntest
  mailcommon
  KF5::KDELibs4Support
  Qt5::Test
  Qt5::Gui
)
