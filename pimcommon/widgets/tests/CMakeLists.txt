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
