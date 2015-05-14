
set(selectiontreewidget_gui_SRCS selectiontreewidget_gui.cpp ../gui/dialog/selectiontypedialog.cpp ../gui/widgets/selectiontypetreewidget.cpp ../utils.cpp ../xml/templateselection.cpp ../pimsettingexport_debug.cpp )
add_executable(selectiontreewidget_gui ${selectiontreewidget_gui_SRCS})
target_link_libraries(selectiontreewidget_gui
  KF5::AkonadiCore
  KF5::AkonadiPrivate
  mailcommon
  pimcommon
)

set(showarchivestructuredialog_gui_SRCS showarchivestructuredialog_gui.cpp ../gui/dialog/showarchivestructuredialog.cpp ../utils.cpp ../xml/templateselection.cpp ../pimsettingexport_debug.cpp)
add_executable(showarchivestructuredialog_gui ${showarchivestructuredialog_gui_SRCS})
target_link_libraries(showarchivestructuredialog_gui
  KF5::AkonadiCore
  KF5::AkonadiPrivate
  mailcommon
  pimcommon
)
