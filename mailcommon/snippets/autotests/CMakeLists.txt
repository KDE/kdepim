include_directories(
  ${CMAKE_SOURCE_DIR}/mailcommon
)

set( mailcommon_snippsettest_source snippetsmanagertest.cpp )
add_executable( snippsettest ${mailcommon_snippsettest_source})
add_test(snippsettest snippsettest)
ecm_mark_as_test(snippsettest)
target_link_libraries( snippsettest Qt5::Test  Qt5::Gui mailcommon)
