project(grantleethemeeditor)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon/
)


set(libgrantleethemeeditor_SRCS 
    newthemedialog.cpp 
    themeeditortabwidget.cpp 
    managethemes.cpp 
    themesession.cpp 
    editorwidget.cpp 
    desktopfilepage.cpp 
    editorpage.cpp
    previewwidget.cpp
)


kde4_add_library(grantleethemeeditor ${LIBRARY_TYPE} ${libgrantleethemeeditor_SRCS})

target_link_libraries(grantleethemeeditor  ${QT_QTCORE_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS} pimcommon
)

set_target_properties(grantleethemeeditor PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS grantleethemeeditor ${INSTALL_TARGETS_DEFAULT_ARGS})
