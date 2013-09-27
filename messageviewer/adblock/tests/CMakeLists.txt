include_directories(
  ${CMAKE_SOURCE_DIR}/messageviewer
  ${CMAKE_BINARY_DIR}/messageviewer
)

set(adblocksettings_gui_SRCS adblocksettings_gui.cpp)
kde4_add_executable(adblocksettings_gui TEST ${adblocksettings_gui_SRCS})
target_link_libraries(adblocksettings_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  messageviewer
)

