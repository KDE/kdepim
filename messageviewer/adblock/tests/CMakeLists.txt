include_directories(
  ${CMAKE_SOURCE_DIR}/messageviewer
  ${CMAKE_BINARY_DIR}/messageviewer
)

if(NOT KDEPIM_NO_WEBKIT)
set(adblocksettings_gui_SRCS adblocksettings_gui.cpp)
kde4_add_executable(adblocksettings_gui TEST ${adblocksettings_gui_SRCS})
target_link_libraries(adblocksettings_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  messageviewer
)

set(adblocksettingsdialog_gui_SRCS adblocksettingsdialog_gui.cpp)
kde4_add_executable(adblocksettingsdialog_gui TEST ${adblocksettingsdialog_gui_SRCS})
target_link_libraries(adblocksettingsdialog_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  messageviewer
)

set(adblockblockableitemdialog_gui_SRCS adblockblockableitemdialog_gui.cpp)
kde4_add_executable(adblockblockableitemdialog_gui TEST ${adblockblockableitemdialog_gui_SRCS})
target_link_libraries(adblockblockableitemdialog_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  ${QT_QTWEBKIT_LIBRARY}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  messageviewer
)
endif()
