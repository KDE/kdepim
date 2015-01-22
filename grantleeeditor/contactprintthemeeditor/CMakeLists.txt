project( contactprintthemeeditor )

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )



include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon/
  ${CMAKE_SOURCE_DIR}/grantleeeditor/grantleethemeeditor
  ${CMAKE_SOURCE_DIR}/kaddressbookgrantlee
  ${CMAKE_SOURCE_DIR}/grantleeeditor/
)


set(contactprintthemeeditor_SRCS
  main.cpp
  defaultcompletion.cpp
  editorpage.cpp
  themeconfiguredialog.cpp
  themedefaulttemplate.cpp
  themeeditormainwindow.cpp
  themeeditorpage.cpp
  themeeditorutil.cpp
  themeeditorwidget.cpp
  themetemplatewidget.cpp
  contactprintthemepreview.cpp
)


add_executable(contactprintthemeeditor ${contactprintthemeeditor_SRCS} )
TARGET_LINK_LIBRARIES(contactprintthemeeditor 
                       KF5::Mime 
                       KF5::DBusAddons                        
                       KF5::PimTextEdit 
                       KF5::KIOCore 
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
