include_directories(${CMAKE_SOURCE_DIR}/mailcommon
)


macro(add_mailcommon_widget_unittest _source)
  set(_test ${_source} )
  get_filename_component(_name ${_source} NAME_WE)
  kde4_add_unit_test(${_name} TESTNAME mailcommon-widget-${_name} ${_test} )
  target_link_libraries( ${_name}
    mailcommon
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
  )
endmacro ()

add_mailcommon_widget_unittest(redirectdialogtest.cpp)
