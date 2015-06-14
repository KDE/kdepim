project( contactprintthemeeditor )




include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon/
  ${CMAKE_SOURCE_DIR}/grantleeeditor/grantleethemeeditor
  ${CMAKE_SOURCE_DIR}/kaddressbookgrantlee
  ${CMAKE_SOURCE_DIR}/grantleeeditor/
)


set(contactprintthemeeditor_SRCS
  main.cpp
  contactprintthemeeditor_debug.cpp
  defaultcompletion.cpp
  editorpage.cpp
  contactprintthemeconfiguredialog.cpp
  themedefaulttemplate.cpp
  themeeditormainwindow.cpp
  themeeditorpage.cpp
  contactprintthemeeditorutil.cpp
  themeeditorwidget.cpp
  themetemplatewidget.cpp
  contactprintthemepreview.cpp
)


add_executable(contactprintthemeeditor ${contactprintthemeeditor_SRCS} )
target_link_libraries(contactprintthemeeditor 
                       KF5::DBusAddons                        
                       KF5::NewStuff 
                       KF5::AkonadiContact
                       pimcommon 
                       grantleethemeeditor 
                       kaddressbookgrantlee
)

install(TARGETS contactprintthemeeditor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  contactprintthemeeditorui.rc
  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/contactprintthemeeditor)

install(PROGRAMS org.kde.contactprintthemeeditor.desktop DESTINATION ${KDE_INSTALL_APPDIR})
