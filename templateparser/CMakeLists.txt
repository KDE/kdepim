project(templateparser)
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

kde_enable_exceptions()

set(templateparser_LIB_SRCS
  templateparser.cpp
  defaulttemplates.cpp
  templatesutil.cpp
  customtemplates.cpp
  customtemplatesmenu.cpp
  templatesconfiguration.cpp
  templatesinsertcommand.cpp
  templatessyntaxhighlighter.cpp
  templatestextedit.cpp
  templatestexteditor.cpp
)

include_directories(
  ${Boost_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/libkleo
  ${CMAKE_SOURCE_DIR}/messagecore
  ${CMAKE_SOURCE_DIR}/messageviewer
  ${CMAKE_SOURCE_DIR}/pimcommon
)

kconfig_add_kcfg_files(templateparser_LIB_SRCS settings/templatesconfiguration_kfg.kcfgc)

kconfig_add_kcfg_files(templateparser_LIB_SRCS
  settings/customtemplates_kfg.kcfgc
  settings/globalsettings_base.kcfgc
)

qt5_wrap_ui(templateparser_LIB_SRCS
  ui/templatesconfiguration_base.ui
  ui/customtemplates_base.ui
)

add_library(templateparser ${LIBRARY_TYPE} ${templateparser_LIB_SRCS})
target_link_libraries(templateparser
  messageviewer
  KF5::KIOCore
  KF5::Mime
  KF5::AkonadiCore
  KF5::PimTextEdit
  KF5::PimIdentities
  messagecore
  kleo
  kpgp
  pimcommon
  ${QGPGME_LIBRARIES}
  ${QT_QTWEBKIT_LIBRARY}
)
set_target_properties(templateparser PROPERTIES
  VERSION ${KDEPIM_LIB_VERSION}
  SOVERSION ${KDEPIM_LIB_SOVERSION}
)

install(TARGETS templateparser ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  settings/customtemplates_kfg.kcfg
  settings/templatesconfiguration_kfg.kcfg
  DESTINATION ${KCFG_INSTALL_DIR}
)

#REACTIVATE
#add_subdirectory(tests)
