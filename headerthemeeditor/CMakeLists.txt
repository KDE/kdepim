project( headerthemeeditor )

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


set(headerthemeeditor_SRCS
  main.cpp
  editor.cpp  
  themeeditormainwindow.cpp 
  themeeditorpage.cpp
  previewpage.cpp
  desktopfilepage.cpp
  editorpage.cpp
  themetemplatewidget.cpp
  themedefaulttemplate.cpp
  defaultcompletion.cpp
  newthemedialog.cpp
  themesession.cpp
)


kde4_add_executable(headerthemeeditor ${headerthemeeditor_SRCS} )
TARGET_LINK_LIBRARIES(headerthemeeditor ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KNEWSTUFF3_LIBS} pimcommon messageviewer)

install(TARGETS headerthemeeditor ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  headerthemeeditorui.rc
  DESTINATION  ${DATA_INSTALL_DIR}/headerthemeeditor)

install(PROGRAMS headerthemeeditor.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
