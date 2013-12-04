include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(webdav_gui_SRCS webdav_gui.cpp )
kde4_add_executable(webdav_gui TEST ${webdav_gui_SRCS})
target_link_libraries(webdav_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  pimcommon
)

