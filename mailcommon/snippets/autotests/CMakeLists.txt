include_directories(
  ${CMAKE_SOURCE_DIR}/mailcommon
)

set( mailcommon_snippsettest_source snippetsmanagertest.cpp )
kde4_add_unit_test( snippsettest ${mailcommon_snippsettest_source})
target_link_libraries( snippsettest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} mailcommon)
