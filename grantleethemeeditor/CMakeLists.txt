project(grantleethemeeditor)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

set(libgrantleethemeeditor_SRCS newthemedialog.cpp themeeditortabwidget.cpp managethemes.cpp 
)


kde4_add_library(grantleethemeeditor ${LIBRARY_TYPE} ${libgrantleethemeeditor_SRCS})

target_link_libraries(grantleethemeeditor  ${QT_QTCORE_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS}
)

set_target_properties(grantleethemeeditor PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS grantleethemeeditor ${INSTALL_TARGETS_DEFAULT_ARGS})
