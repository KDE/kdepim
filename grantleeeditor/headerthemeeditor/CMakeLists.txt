project( headerthemeeditor )



include_directories(
  ${CMAKE_SOURCE_DIR}/messageviewer
  ${CMAKE_SOURCE_DIR}/pimcommon/
  ${CMAKE_SOURCE_DIR}/grantleeeditor/grantleethemeeditor
  ${CMAKE_SOURCE_DIR}/grantleeeditor/
)


set(headerthemeeditor_SRCS
  main.cpp
  headerthemeeditor_debug.cpp
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


add_executable(headerthemeeditor ${headerthemeeditor_SRCS} )
target_link_libraries(headerthemeeditor KF5::Mime  KF5::NewStuff pimcommon messageviewer grantleethemeeditor KF5::DBusAddons grantleetheme)

install(TARGETS headerthemeeditor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  headerthemeeditorui.rc
  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/headerthemeeditor)

install(PROGRAMS org.kde.headerthemeeditor.desktop DESTINATION ${KDE_INSTALL_APPDIR})
