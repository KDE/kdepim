include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon
)

set(pimcommon_util_editorutiltest_SRCS editorutiltest.cpp ../editorutil.cpp)
kde4_add_unit_test( editorutiltest ${pimcommon_util_editorutiltest_SRCS} )
target_link_libraries( editorutiltest
   ${QT_QTTEST_LIBRARY}
   ${QT_QTGUI_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDE4_KDECORE_LIBS}
   pimcommon
)
