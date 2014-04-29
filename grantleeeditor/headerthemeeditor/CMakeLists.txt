project( headerthemeeditor )

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

kde_enable_exceptions()

include_directories(
  ${CMAKE_SOURCE_DIR}/messageviewer
  ${CMAKE_SOURCE_DIR}/pimcommon/
  ${CMAKE_SOURCE_DIR}/grantleeeditor/grantleethemeeditor
  ${CMAKE_SOURCE_DIR}/grantleeeditor/
)


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


add_executable(headerthemeeditor ${headerthemeeditor_SRCS} )
TARGET_LINK_LIBRARIES(headerthemeeditor KF5::Mime  KF5::PimTextEdit KF5::KIOCore KF5::NewStuff pimcommon messageviewer grantleethemeeditor KF5::DBusAddons)

install(TARGETS headerthemeeditor ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  headerthemeeditorui.rc
  DESTINATION  ${DATA_INSTALL_DIR}/headerthemeeditor)

install(PROGRAMS headerthemeeditor.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
