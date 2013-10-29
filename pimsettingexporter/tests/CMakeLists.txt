include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(selectiontreewidget_gui_SRCS selectiontreewidget_gui.cpp ../selectiontypedialog.cpp ../selectiontypetreewidget.cpp )
kde4_add_executable(selectiontreewidget_gui TEST ${selectiontreewidget_gui_SRCS})
target_link_libraries(selectiontreewidget_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
)

