set(autocreatescripts_gui_SRCS autocreatescripts_gui.cpp)
kde4_add_executable(autocreatescripts_gui TEST ${autocreatescripts_gui_SRCS})
target_link_libraries(autocreatescripts_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ksieveui
  pimcommon
)
