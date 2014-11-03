include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(baloodebug_dialog_gui_SRCS baloodebug_dialog_gui.cpp)
kde4_add_executable(baloodebug_dialog_gui TEST ${baloodebug_dialog_gui_SRCS})
target_link_libraries(baloodebug_dialog_gui
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  pimcommon
)

