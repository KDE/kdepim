include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(gravatarwidget_gui_SRCS gravatarwidget_gui.cpp)
kde4_add_executable(gravatarwidget_gui TEST ${gravatarwidget_gui_SRCS})
target_link_libraries(gravatarwidget_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  pimcommon
)

