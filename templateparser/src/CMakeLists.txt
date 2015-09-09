add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


add_definitions(-DTRANSLATION_DOMAIN=\"libtemplateparser\")


set(templateparser_LIB_SRCS
  templateparser.cpp
  defaulttemplates.cpp
  templatesutil.cpp
  customtemplates.cpp
  customtemplatesmenu.cpp
  templatesconfiguration.cpp
  templatesinsertcommand.cpp
  templatestextedit.cpp
  templatestexteditor.cpp
  templatessyntaxhighlighterrules.cpp
)

include_directories(
  ${CMAKE_SOURCE_DIR}/libkleo
  ${CMAKE_SOURCE_DIR}/messageviewer
)

ecm_qt_declare_logging_category(templateparser_LIB_SRCS HEADER templateparser_debug.h IDENTIFIER TEMPLATEPARSER_LOG CATEGORY_NAME log_templateparser)

kconfig_add_kcfg_files(templateparser_LIB_SRCS settings/templatesconfiguration_kfg.kcfgc)

kconfig_add_kcfg_files(templateparser_LIB_SRCS
  settings/customtemplates_kfg.kcfgc
  settings/globalsettings_templateparser.kcfgc
)

ki18n_wrap_ui(templateparser_LIB_SRCS
  ui/templatesconfiguration_base.ui
  ui/customtemplates_base.ui
)

add_library(KF5TemplateParser ${templateparser_LIB_SRCS})
generate_export_header(KF5TemplateParser BASE_NAME templateparser)
add_library(KF5::TemplateParser ALIAS KF5TemplateParser)

target_link_libraries(KF5TemplateParser
PUBLIC
  pimcommon
PRIVATE
  KF5::Mime
  KF5::AkonadiCore
  messageviewer
  KF5::IdentityManagement
  messagecore
  Qt5::WebKitWidgets
  KF5::XmlGui
  KF5::IconThemes
  KF5::Completion
)
set_target_properties(KF5TemplateParser PROPERTIES
  VERSION ${KDEPIM_LIB_VERSION}
  SOVERSION ${KDEPIM_LIB_SOVERSION}
  EXPORT_NAME TemplateParser
)

target_include_directories(KF5TemplateParser PUBLIC "$<BUILD_INTERFACE:${templateparser_SOURCE_DIR};${templateparser_BINARY_DIR}>")
target_include_directories(KF5TemplateParser PUBLIC "$<BUILD_INTERFACE:${pimcommon_SOURCE_DIR}/src;${pimcommon_BINARY_DIR}/src>")



install(FILES
  settings/customtemplates_kfg.kcfg
  settings/templatesconfiguration_kfg.kcfg
  DESTINATION ${KDE_INSTALL_KCFGDIR}
)

ecm_generate_headers(TemplateParser_CamelCase_HEADERS
  HEADER_NAMES
  TemplatesTextEdit
  CustomTemplatesMenu
  TemplatesConfiguration
  TemplateParser
  REQUIRED_HEADERS TemplateParser_HEADERS
  PREFIX TemplateParser
)

if(${CMAKE_SOURCE_DIR} STREQUAL ${TemplateParser_SOURCE_DIR})

install(TARGETS KF5TemplateParser EXPORT KF5TemplateParserTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${TemplateParser_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/TemplateParser
    COMPONENT Devel
)

install(FILES
    ${TemplateParser_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/templateparser_export.h
    ${TemplateParser_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/templateparser
    COMPONENT Devel
)

endif()
