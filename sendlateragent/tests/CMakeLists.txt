set(sendlaterinfo_gui_SRCS sendlaterinfo_gui.cpp )
kde4_add_executable(sendlaterinfo_gui TEST ${sendlaterinfo_gui_SRCS})
target_link_libraries(sendlaterinfo_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  sendlater
)
