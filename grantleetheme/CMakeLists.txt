project(grantleetheme)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

set(libgrantleetheme_SRCS grantleetheme.cpp grantleethememanager.cpp
)


kde4_add_kcfg_files(libgrantleetheme_SRCS
  globalsettings_base.kcfgc
)

kde4_add_library(grantleetheme ${LIBRARY_TYPE} ${libgrantleetheme_SRCS})

target_link_libraries(grantleetheme  ${QT_QTCORE_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KNEWSTUFF3_LIBS}
)

set_target_properties(grantleetheme PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS grantleetheme ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(kconf_update)
