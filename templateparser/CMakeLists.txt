project(templateparser)
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
  templateparser_debug.cpp
  templatessyntaxhighlighterrules.cpp
)

include_directories(
  ${CMAKE_SOURCE_DIR}/libkleo
  ${CMAKE_SOURCE_DIR}/messageviewer
)

kconfig_add_kcfg_files(templateparser_LIB_SRCS settings/templatesconfiguration_kfg.kcfgc)

kconfig_add_kcfg_files(templateparser_LIB_SRCS
  settings/customtemplates_kfg.kcfgc
  settings/globalsettings_base.kcfgc
)

ki18n_wrap_ui(templateparser_LIB_SRCS
  ui/templatesconfiguration_base.ui
  ui/customtemplates_base.ui
)

add_library(templateparser ${templateparser_LIB_SRCS})
generate_export_header(templateparser BASE_NAME templateparser)
target_link_libraries(templateparser
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
)
set_target_properties(templateparser PROPERTIES
  VERSION ${KDEPIM_LIB_VERSION}
  SOVERSION ${KDEPIM_LIB_SOVERSION}
)

target_include_directories(templateparser PUBLIC "$<BUILD_INTERFACE:${templateparser_SOURCE_DIR};${templateparser_BINARY_DIR}>")
target_include_directories(templateparser PUBLIC "$<BUILD_INTERFACE:${pimcommon_SOURCE_DIR};${pimcommon_BINARY_DIR}>")


install(TARGETS templateparser ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  settings/customtemplates_kfg.kcfg
  settings/templatesconfiguration_kfg.kcfg
  DESTINATION ${KDE_INSTALL_KCFGDIR}
)

if(BUILD_TESTING)
  add_subdirectory(tests)
  add_subdirectory(autotests)
endif()
