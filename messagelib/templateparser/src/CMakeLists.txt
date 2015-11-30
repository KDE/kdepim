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
  KF5::PimCommon
PRIVATE
  KF5::Mime
  KF5::AkonadiCore
  KF5::MessageViewer
  KF5::IdentityManagement
  KF5::MessageCore
  Qt5::WebKitWidgets
  KF5::XmlGui
  KF5::IconThemes
  KF5::Completion
  KF5::Libkleo
)
set_target_properties(KF5TemplateParser PROPERTIES
  VERSION ${TEMPLATEPARSER_VERSION_STRING}
  SOVERSION ${TEMPLATEPARSER_SOVERSION}
  EXPORT_NAME TemplateParser
)

target_include_directories(KF5TemplateParser INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/TemplateParser/;${KDE_INSTALL_INCLUDEDIR_KF5}/templateparser>")
target_include_directories(KF5TemplateParser PUBLIC "$<BUILD_INTERFACE:${TemplateParser_SOURCE_DIR}/src;${TemplateParser_BINARY_DIR}/src>")

install(FILES
  settings/customtemplates_kfg.kcfg
  settings/templatesconfiguration_kfg.kcfg
  DESTINATION ${KDE_INSTALL_KCFGDIR}
)

ecm_generate_headers(TemplateParser_CamelCase_HEADERS
  HEADER_NAMES
  CustomTemplates
  CustomTemplatesMenu
  DefaultTemplates
  TemplateParser
  TemplatesConfiguration
  TemplatesTextEdit
  TemplatesUtil
  TemplatesInsertCommand
  REQUIRED_HEADERS TemplateParser_HEADERS
  PREFIX TemplateParser
)

ecm_generate_pri_file(BASE_NAME TemplateParser
    LIB_NAME KF5TemplateParser
    DEPS "PimCommon" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/TemplateParser
)


install(TARGETS KF5TemplateParser EXPORT KF5TemplateParserTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} ${LIBRARY_NAMELINK})
if(${CMAKE_SOURCE_DIR} STREQUAL ${Messagelib_SOURCE_DIR})


install(FILES
    ${TemplateParser_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/TemplateParser
    COMPONENT Devel
)

install(FILES
    ${TemplateParser_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/templateparser_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/globalsettings_templateparser.h
    ${CMAKE_CURRENT_BINARY_DIR}/customtemplates_kfg.h
    ${CMAKE_CURRENT_BINARY_DIR}/ui_templatesconfiguration_base.h
    ${CMAKE_CURRENT_BINARY_DIR}/templatesconfiguration_kfg.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/templateparser
    COMPONENT Devel
)

install(FILES
    ${PRI_FILENAME}
    DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

endif()
