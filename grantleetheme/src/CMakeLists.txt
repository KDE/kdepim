add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )
add_definitions(-DTRANSLATION_DOMAIN=\"libgrantleetheme\")

add_subdirectory(plugin)

set(libgrantleetheme_SRCS
    grantleetheme.cpp
    grantleethememanager.cpp
    grantleeki18nlocalizer.cpp
)

configure_file(config-grantleetheme.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-grantleetheme.h)

ecm_qt_declare_logging_category(libgrantleetheme_SRCS HEADER grantleetheme_debug.h IDENTIFIER GRANTLEETHEME_LOG CATEGORY_NAME log_grantleetheme)

add_library(KF5GrantleeTheme ${libgrantleetheme_SRCS})
generate_export_header(KF5GrantleeTheme BASE_NAME grantleetheme)

add_library(KF5::GrantleeTheme ALIAS KF5GrantleeTheme)

target_include_directories(KF5GrantleeTheme INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/GrantleeTheme/;${KDE_INSTALL_INCLUDEDIR_KF5}/grantleetheme>")
target_include_directories(KF5GrantleeTheme PUBLIC "$<BUILD_INTERFACE:${GrantleeTheme_SOURCE_DIR}/src;${GrantleeTheme_BINARY_DIR}/src;>")


target_link_libraries(KF5GrantleeTheme
  PRIVATE
    KF5::NewStuff
    KF5::I18n
    Grantlee5::Templates
)

set_target_properties(KF5GrantleeTheme PROPERTIES
    VERSION ${GRANTLEETHEME_VERSION_STRING}
    SOVERSION ${GRANTLEETHEME_SOVERSION}
    EXPORT_NAME GrantleeTheme
)

install(TARGETS
    KF5GrantleeTheme
    EXPORT KF5GrantleeThemeTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS} ${LIBRARY_NAMELINK}
)

ecm_generate_headers(GrantleeTheme_CamelCase_HEADERS
  HEADER_NAMES
  GrantleeThemeManager
  GrantleeTheme
  REQUIRED_HEADERS GrantleeTheme_HEADERS
  PREFIX GrantleeTheme
)

ecm_generate_pri_file(BASE_NAME GrantleeTheme
    LIB_NAME KF5GrantleeTheme
    DEPS "" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/GrantleeTheme
)


if(${CMAKE_SOURCE_DIR} STREQUAL ${GrantleeTheme_SOURCE_DIR})
install(FILES
    ${GrantleeTheme_HEADERS}
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
install(FILES
    ${PRI_FILENAME}
    DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

endif()

