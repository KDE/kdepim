project(grantleetheme)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )
add_definitions(-DTRANSLATION_DOMAIN=\"libgrantleetheme\")
set(libgrantleetheme_SRCS grantleetheme.cpp grantleethememanager.cpp
)


kconfig_add_kcfg_files(libgrantleetheme_SRCS
  globalsettings_base.kcfgc
)

add_library(grantleetheme ${LIBRARY_TYPE} ${libgrantleetheme_SRCS})
generate_export_header(grantleetheme BASE_NAME grantleetheme)

target_link_libraries(grantleetheme    KF5::NewStuff KF5::I18n )

set_target_properties(grantleetheme PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})
install(TARGETS grantleetheme ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(kconf_update)
