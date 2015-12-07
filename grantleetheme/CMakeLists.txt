cmake_minimum_required(VERSION 2.8.12)

project(GrantleeTheme)

if(${CMAKE_SOURCE_DIR} STREQUAL ${GrantleeTheme_SOURCE_DIR})
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
set(KDEPIM_LIB_VERSION "5.1.42")
set(QT_REQUIRED_VERSION "5.4.0")
set(GRANTLEE_VERSION "5.0.0")

find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Test Network)
find_package(KF5I18n ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5IconThemes ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5NewStuff ${KF5_VERSION} CONFIG REQUIRED)

find_package(Grantlee5 ${GRANTLEE_VERSION} CONFIG REQUIRED)

ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX GRANTLEETHEME
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/grantleetheme_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5GrantleeThemeConfigVersion.cmake"
                        SOVERSION 5
)

########### Targets ###########

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5GrantleeTheme")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5GrantleeThemeConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5GrantleeThemeConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5GrantleeThemeConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5GrantleeThemeConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5GrantleeThemeTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5GrantleeThemeTargets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/grantleetheme_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
else()
ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX GRANTLEETHEME
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/grantleetheme_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5GrantleeThemeConfigVersion.cmake"
                        SOVERSION 5
)
endif()

add_subdirectory(src)
if (BUILD_TESTING)
   add_subdirectory(autotest)
endif()

install( FILES grantleetheme.categories DESTINATION ${KDE_INSTALL_CONFDIR} )
