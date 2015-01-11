project( contactthemeeditor )

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )



include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon/
  ${CMAKE_SOURCE_DIR}/grantleeeditor/grantleethemeeditor
  ${CMAKE_SOURCE_DIR}/kaddressbookgrantlee
  ${CMAKE_SOURCE_DIR}/grantleeeditor/
)


set(contactthemeeditor_SRCS
  main.cpp
  defaultcompletion.cpp
  contactdefaulttemplate.cpp
  contacteditorwidget.cpp
  contacttemplatewidget.cpp
  contacteditormainwindow.cpp
  previewwidget.cpp
  editorpage.cpp
  contacteditorpage.cpp
  contactconfigurationdialog.cpp
  contacteditorutil.cpp
  contactpreviewwidget.cpp
)


add_executable(contactthemeeditor ${contactthemeeditor_SRCS} )
TARGET_LINK_LIBRARIES(contactthemeeditor 
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

install(TARGETS contactthemeeditor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  contactthemeeditorui.rc
  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/contactthemeeditor)

install(PROGRAMS org.kde.contactthemeeditor.desktop DESTINATION ${KDE_INSTALL_APPDIR})
