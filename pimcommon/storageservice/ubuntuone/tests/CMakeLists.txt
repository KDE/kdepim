include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(ubuntuone_gui_SRCS ubuntuone_gui.cpp ../../tests/servicetestwidget.cpp)
kde4_add_executable(ubuntuone_gui TEST ${ubuntuone_gui_SRCS})
target_link_libraries(ubuntuone_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  pimcommon
)

