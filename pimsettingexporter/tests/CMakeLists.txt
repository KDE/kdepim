include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(selectiontreewidget_gui_SRCS selectiontreewidget_gui.cpp ../dialog/selectiontypedialog.cpp ../widgets/selectiontypetreewidget.cpp ../utils.cpp )
kde4_add_executable(selectiontreewidget_gui TEST ${selectiontreewidget_gui_SRCS})
target_link_libraries(selectiontreewidget_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${AKONADI_COMMON_LIBRARIES}
  mailcommon
)

set(showarchivestructuredialog_gui_SRCS showarchivestructuredialog_gui.cpp ../dialog/showarchivestructuredialog.cpp ../utils.cpp)
kde4_add_executable(showarchivestructuredialog_gui TEST ${showarchivestructuredialog_gui_SRCS})
target_link_libraries(showarchivestructuredialog_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KFILE_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${AKONADI_COMMON_LIBRARIES}
  mailcommon
)
