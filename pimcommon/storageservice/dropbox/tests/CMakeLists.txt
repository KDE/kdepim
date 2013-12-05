include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(dropboxtoken_gui_SRCS dropboxtoken_gui.cpp ../../tests/servicetestwidget.cpp )
kde4_add_executable(dropboxtoken_gui TEST ${dropboxtoken_gui_SRCS})
target_link_libraries(dropboxtoken_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  pimcommon
)

