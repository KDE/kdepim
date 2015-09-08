add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )
add_definitions(-DTRANSLATION_DOMAIN=\"libgrantleetheme\")

set(libgrantleetheme_SRCS
    grantleetheme.cpp
    grantleethememanager.cpp
)

kconfig_add_kcfg_files(libgrantleetheme_SRCS
  globalsettings_grantleetheme.kcfgc
)

add_library(KF5GrantleeTheme ${libgrantleetheme_SRCS})
generate_export_header(KF5GrantleeTheme BASE_NAME grantleetheme)

add_library(KF5::GrantleeTheme ALIAS KF5GrantleeTheme)

target_include_directories(KF5GrantleeTheme INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/GrantleeTheme/;${KDE_INSTALL_INCLUDEDIR_KF5}/grantleetheme>")
target_include_directories(KF5GrantleeTheme PUBLIC "$<BUILD_INTERFACE:${GrantleeTheme_SOURCE_DIR}/src;${GrantleeTheme_BINARY_DIR}/src;>")


target_link_libraries(KF5GrantleeTheme
PRIVATE
  KF5::NewStuff KF5::I18n
)

set_target_properties(KF5GrantleeTheme PROPERTIES
    VERSION ${GRANTLEETHEME_VERSION_STRING}
    SOVERSION ${GRANTLEETHEME_SOVERSION}
    EXPORT_NAME GrantleeTheme
)

install(TARGETS
    KF5GrantleeTheme
    EXPORT KF5GrantleeThemeTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS}
)

ecm_generate_headers(GrantleeTheme_CamelCase_HEADERS
  HEADER_NAMES
  GrantleeThemeManager
  GrantleeTheme
  #Theme
  REQUIRED_HEADERS GrantleeTheme_HEADERS
  PREFIX GrantleeTheme
)

if(${CMAKE_SOURCE_DIR} STREQUAL ${GrantleeTheme_SOURCE_DIR})
install(FILES
    ${GrantleeTheme_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/GrantleeTheme
    COMPONENT Devel
)

install(FILES
    ${GrantleeTheme_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/grantleetheme_export.h
    ${GrantleeTheme_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/grantleetheme
    COMPONENT Devel
)
endif()

