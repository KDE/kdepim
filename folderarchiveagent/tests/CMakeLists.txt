set(folderarchive_gui_SRCS folderarchive_gui.cpp ../folderarchiveconfiguredialog.cpp ../folderarchivesettingpage.cpp)
kde4_add_executable(folderarchive_gui TEST ${folderarchive_gui_SRCS})
target_link_libraries(folderarchive_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  mailcommon
)
