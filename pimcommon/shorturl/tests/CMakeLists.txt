include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(shorturlwidget_gui_SRCS shorturlwidget_gui.cpp)
kde4_add_executable(shorturlwidget_gui TEST ${shorturlwidget_gui_SRCS})
target_link_libraries(shorturlwidget_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  pimcommon
)

