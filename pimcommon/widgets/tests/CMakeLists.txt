include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(customtextedit_gui_SRCS customtextedit_gui.cpp)
kde4_add_executable(customtextedit_gui TEST ${customtextedit_gui_SRCS})
target_link_libraries(customtextedit_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  pimcommon
)

set(customtoolswidget_gui_SRCS customtoolswidget_gui.cpp)
kde4_add_executable(customtoolswidget_gui TEST ${customtoolswidget_gui_SRCS})
target_link_libraries(customtoolswidget_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  pimcommon
)

set(slidecontainer_gui_SRCS slidecontainer_gui.cpp)
kde4_add_executable(slidecontainer_gui TEST ${slidecontainer_gui_SRCS})
target_link_libraries(slidecontainer_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  pimcommon
)


macro(add_pimcommon_widget_unittest _source)
  set(_test ${_source} )
  get_filename_component(_name ${_source} NAME_WE)
  kde4_add_unit_test(${_name} TESTNAME pimcommon-widget-${_name} ${_test} )
  target_link_libraries( ${_name}
    pimcommon
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
  )
endmacro ()

add_pimcommon_widget_unittest(slidecontainertest.cpp)
