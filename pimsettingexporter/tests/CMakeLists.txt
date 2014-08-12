include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(selectiontreewidget_gui_SRCS selectiontreewidget_gui.cpp ../dialog/selectiontypedialog.cpp ../widgets/selectiontypetreewidget.cpp ../utils.cpp ../xml/templateselection.cpp )
add_executable(selectiontreewidget_gui ${selectiontreewidget_gui_SRCS})
target_link_libraries(selectiontreewidget_gui
  KF5::AkonadiCore
  mailcommon
  pimcommon
)

set(showarchivestructuredialog_gui_SRCS showarchivestructuredialog_gui.cpp ../dialog/showarchivestructuredialog.cpp ../utils.cpp ../xml/templateselection.cpp)
add_executable(showarchivestructuredialog_gui ${showarchivestructuredialog_gui_SRCS})
target_link_libraries(showarchivestructuredialog_gui
  KF5::AkonadiCore
  mailcommon
  pimcommon
)
