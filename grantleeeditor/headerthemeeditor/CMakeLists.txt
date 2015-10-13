project( headerthemeeditor )

set(headerthemeeditor_SRCS
  main.cpp
  themeeditormainwindow.cpp 
  themeeditorpage.cpp
  previewwidget.cpp
  editorpage.cpp
  themetemplatewidget.cpp
  themedefaulttemplate.cpp
  defaultcompletion.cpp
  themeconfiguredialog.cpp
  themeeditorutil.cpp
  themeeditorwidget.cpp
)

ecm_qt_declare_logging_category(headerthemeeditor_SRCS HEADER headerthemeeditor_debug.h IDENTIFIER HEADERTHEMEEDITOR_LOG CATEGORY_NAME log_headerthemeeditor)
qt5_add_resources(headerthemeeditor_SRCS headerthemeeditor.qrc)

add_executable(headerthemeeditor ${headerthemeeditor_SRCS} )
target_link_libraries(headerthemeeditor KF5::Mime  KF5::NewStuff KF5::PimCommon KF5::MessageViewer grantleethemeeditor KF5::DBusAddons KF5::GrantleeTheme)

install(TARGETS headerthemeeditor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS org.kde.headerthemeeditor.desktop DESTINATION ${KDE_INSTALL_APPDIR})
