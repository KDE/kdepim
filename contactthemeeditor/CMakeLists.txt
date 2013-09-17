project( contactthemeeditor )

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon/
  ${CMAKE_SOURCE_DIR}/grantleethemeeditor
  ${CMAKE_SOURCE_DIR}/kaddressbookgrantlee
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


kde4_add_executable(contactthemeeditor ${contactthemeeditor_SRCS} )
TARGET_LINK_LIBRARIES(contactthemeeditor 
                       ${KDEPIMLIBS_KMIME_LIBS} 
                       ${KDE4_KDEUI_LIBS} 
                       ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS} 
                       ${KDE4_KIO_LIBS} 
                       ${KDE4_KNEWSTUFF3_LIBS} 
                       ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
                       pimcommon 
                       grantleethemeeditor 
                       kaddressbookgrantlee
)

install(TARGETS contactthemeeditor ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  contactthemeeditorui.rc
  DESTINATION  ${DATA_INSTALL_DIR}/contactthemeeditor)

install(PROGRAMS contactthemeeditor.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
