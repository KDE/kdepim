project( headerthemeeditor )


set(headerthemeeditor_SRCS
  main.cpp
  themeeditormainwindow.cpp 
  themeeditorpage.cpp
  previewpage.cpp
  desktopfilepage.cpp
  editorpage.cpp
  themetemplatewidget.cpp
)

kde4_add_executable(headerthemeeditor ${headerthemeeditor_SRCS} )
TARGET_LINK_LIBRARIES(headerthemeeditor ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS} pimcommon )

install(TARGETS headerthemeeditor ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  headerthemeeditorui.rc
  DESTINATION  ${DATA_INSTALL_DIR}/headerthemeeditor)

install(PROGRAMS headerthemeeditor.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
