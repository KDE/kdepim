cmake_minimum_required(VERSION 2.8.12)

project(kaddressbookgrantlee)

if(${CMAKE_SOURCE_DIR} STREQUAL ${kaddressbookgrantlee_SOURCE_DIR})
# ECM setup
find_package(ECM 5.16.0 CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
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
set(KDEPIM_LIB_VERSION "5.0.47")
set(QT_REQUIRED_VERSION "5.4.0")
set(KCONTACTS_LIB_VERSION "4.92.0")

find_package(KF5Akonadi ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Test UiTools)
find_package(KF5I18n ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5IconThemes ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Config ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Service ${KF5_VERSION} CONFIG REQUIRED)
find_package(Grantlee5 "5.0" CONFIG REQUIRED)
find_package(KF5Libkleo ${KDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5GrantleeTheme ${KDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiContact ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Contacts ${KCONTACTS_LIB_VERSION} CONFIG REQUIRED)


ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX KADDRESSBOOKGRANTLEE
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kaddressbookgrantlee_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5KaddressbookGrantleeConfigVersion.cmake"
                        SOVERSION 5
)

########### Targets ###########

add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5KaddressbookGrantlee")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5KaddressbookGrantleeConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5KaddressbookGrantleeConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5KaddressbookGrantleeConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5KaddressbookGrantleeConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5KaddressbookGrantleeTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5KaddressbookGrantleeTargets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kaddressbookgrantlee_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
else()
ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX KADDRESSBOOKGRANTLEE
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kaddressbookgrantlee_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5KaddressbookGrantleeConfigVersion.cmake"
                        SOVERSION 5
)
endif()

add_subdirectory(src)

