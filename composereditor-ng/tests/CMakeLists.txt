PROJECT( composerhtmleditor )

set(composerhtmlSources
  main.cpp
  composerhtmleditor.cpp
)

KDE4_ADD_EXECUTABLE(composerhtmleditor ${composerhtmlSources} )
TARGET_LINK_LIBRARIES(composerhtmleditor ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} composereditorng )

install(TARGETS composerhtmleditor ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  composerhtmleditorui.rc
  DESTINATION  ${DATA_INSTALL_DIR}/composerhtmleditor)

