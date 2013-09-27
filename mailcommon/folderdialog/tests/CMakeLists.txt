
set(selectmultiselectiondialog_gui_SRCS selectmultiselectiondialog_gui.cpp)
kde4_add_executable(selectmultiselectiondialog_gui TEST ${selectmultiselectiondialog_gui_SRCS})
target_link_libraries(selectmultiselectiondialog_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  mailcommon
)

