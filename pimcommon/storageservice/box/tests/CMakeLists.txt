include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(box_gui_SRCS box_gui.cpp ../../tests/servicetestwidget.cpp)
kde4_add_executable(box_gui TEST ${box_gui_SRCS})
target_link_libraries(box_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  pimcommon
)

