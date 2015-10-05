cmake_minimum_required(VERSION 2.8.12)

project(sendlater)

if(${CMAKE_SOURCE_DIR} STREQUAL ${sendlater_SOURCE_DIR})
# ECM setup
find_package(ECM 5.14.0 CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

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


set(KF5_VERSION "5.14.0")
set(KDEPIMLIBS_LIB_VERSION "5.0.46")
set(KDEPIM_LIB_VERSION "5.43.0")
set(QT_REQUIRED_VERSION "5.4.0")

find_package(KF5Akonadi ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Test UiTools DBus)
find_package(KF5I18n ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Config ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5WidgetsAddons ${KF5_VERSION} CONFIG REQUIRED)



ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX SENDLATER
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/sendlater_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5SendLaterConfigVersion.cmake"
                        SOVERSION 5
)


########### Targets ###########

add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5SendLater")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5SendLaterConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5SendLaterConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5SendLaterConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5SendLaterConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5SendLaterTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5SendLaterTargets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/sendlater_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
else()
ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX SENDLATER
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/sendlater_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5SendLaterConfigVersion.cmake"
                        SOVERSION 5
)
endif()

add_subdirectory(src)

