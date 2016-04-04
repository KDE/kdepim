include_directories( 
   ${CMAKE_SOURCE_DIR}/pimsettingexporter/core/ 
   ${CMAKE_BINARY_DIR}/pimsettingexporter/core/
)

set(selectiontreewidget_gui_SRCS selectiontreewidget_gui.cpp ../dialog/selectiontypedialog.cpp ../widgets/selectiontypetreewidget.cpp ../pimsettingexportgui_debug.cpp)
add_executable(selectiontreewidget_gui ${selectiontreewidget_gui_SRCS})
target_link_libraries(selectiontreewidget_gui
  KF5::AkonadiCore
  KF5::AkonadiPrivate
  KF5::MailCommon
  KF5::PimCommon
  KF5::Archive
  pimsettingexporterprivate
  KF5::ItemViews
  KF5::I18n
)

set(showarchivestructuredialog_gui_SRCS showarchivestructuredialog_gui.cpp ../dialog/showarchivestructuredialog.cpp ../pimsettingexportgui_debug.cpp)
add_executable(showarchivestructuredialog_gui ${showarchivestructuredialog_gui_SRCS})
target_link_libraries(showarchivestructuredialog_gui
  KF5::AkonadiCore
  KF5::AkonadiPrivate
  KF5::MailCommon
  KF5::PimCommon
  KF5::Archive
  pimsettingexporterprivate
  KF5::ItemViews
  KF5::I18n
)
