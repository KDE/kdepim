cmake_minimum_required(VERSION 2.8.12)

project(TemplateParser)


if(${CMAKE_SOURCE_DIR} STREQUAL ${TemplateParser_SOURCE_DIR})
# ECM setup
find_package(ECM 5.16.0 CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${TemplateParser_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})
set(LIBRARY_NAMELINK)
include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings)
include(ECMQtDeclareLoggingCategory)
include(ECMAddTests)


set(KF5_VERSION "5.16.0")
set(KDEPIMLIBS_LIB_VERSION "5.0.48")
set(KDEPIM_LIB_VERSION "5.0.48")
set(QT_REQUIRED_VERSION "5.4.0")
set(KMIME_LIB_VERSION "4.87.0")
set(IDENTITYMANAGEMENT_LIB_VERSION "4.80.0")

find_package(KF5Akonadi ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Test UiTools WebKitWidgets)
find_package(KF5I18n ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Mime ${KMIME_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiMime ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5XmlGui ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5IconThemes ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Completion ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5PimCommon ${KDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5MessageViewer ${KDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5MessageCore ${KDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5IdentityManagement ${IDENTITYMANAGEMENT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Libkleo ${KDEPIM_LIB_VERSION} CONFIG REQUIRED)
ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX TEMPLATEPARSER
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/templateparser_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5TemplateParserConfigVersion.cmake"
                        SOVERSION 5
)

remove_definitions( -DQT_NO_CAST_FROM_BYTEARRAY )

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5TemplateParser")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5TemplateParserConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5TemplateParserConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5TemplateParserConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5TemplateParserConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5TemplateParserTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5TemplateParserTargets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/templateparser_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
else()
ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX TEMPLATEPARSER
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/mailimporter_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5MailImporterConfigVersion.cmake"
                        SOVERSION 5
)
endif()

add_subdirectory(src)
if (BUILD_TESTING)
   add_subdirectory(autotests)
   add_subdirectory(tests)
endif()

