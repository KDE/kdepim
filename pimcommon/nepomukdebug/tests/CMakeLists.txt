include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(searchdebugnepomukshowdialog_gui_SRCS searchdebugnepomukshowdialog_gui.cpp)
kde4_add_executable(searchdebugnepomukshowdialog_gui TEST ${searchdebugnepomukshowdialog_gui_SRCS})
target_link_libraries(searchdebugnepomukshowdialog_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  pimcommon
)

