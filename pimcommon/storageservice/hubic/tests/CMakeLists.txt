include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(hubic_gui_SRCS hubic_gui.cpp )
kde4_add_executable(hubic_gui TEST ${hubic_gui_SRCS})
target_link_libraries(hubic_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  pimcommon
)

