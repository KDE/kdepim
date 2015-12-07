
cmake_minimum_required(VERSION 2.8.12)

project(kdgantt2)

if(${CMAKE_SOURCE_DIR} STREQUAL ${kdgantt2_SOURCE_DIR})
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
set(KMIME_LIB_VERSION "4.87.0")

find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Test PrintSupport)
find_package(KF5I18n ${KF5_VERSION} CONFIG REQUIRED)

ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX KDGANTT2
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdgantt2_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5KDGantt2ConfigVersion.cmake"
                        SOVERSION 5
)

########### Targets ###########

add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)
########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5KDGantt2")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5KDGantt2Config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5KDGantt2Config.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5KDGantt2Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5KDGantt2ConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5KDGantt2Targets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5KDGantt2Targets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kdgantt2_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
else()
ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX KDGANTT2
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdgantt2_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5KDGantt2ConfigVersion.cmake"
                        SOVERSION 5
)
endif()

# Laurent install it if you want to debug it
#add_subdirectory(tests)
add_subdirectory(src)

install( FILES kdgantt2.categories DESTINATION ${KDE_INSTALL_CONFDIR} )
