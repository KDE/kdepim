cmake_minimum_required(VERSION 2.8.12)

project(libksieve)

if(${CMAKE_SOURCE_DIR} STREQUAL ${libksieve_SOURCE_DIR})
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
set(KDEPIMLIBS_LIB_VERSION "5.1.42")
set(KDEPIM_LIB_VERSION "5.1.42")
set(QT_REQUIRED_VERSION "5.4.0")
set(KMIME_LIB_VERSION "4.87.0")
set(IDENTITYMANAGEMENT_LIB_VERSION "4.80.0")
set(KMAILTRANSPORT_LIB_VERSION "4.78.0")
set(KPIMTEXTEDIT_LIB_VERSION "5.1.42")


find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Test UiTools Xml WebKitWidgets)
find_package(KF5I18n ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5IconThemes ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5PimCommon ${KDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Libkdepim ${KDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5NewStuff ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5WindowSystem ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Archive ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Mime ${KMIME_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5IdentityManagement ${IDENTITYMANAGEMENT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5MailTransport ${KMAILTRANSPORT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Akonadi ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5PimTextEdit ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)


ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX LIBKSIEVE
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/libksieve_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5LibKSieveConfigVersion.cmake"
                        SOVERSION 5
)

########### Targets ###########

#add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")
remove_definitions( -DQT_NO_CAST_FROM_BYTEARRAY )
remove_definitions( -DQT_NO_CAST_FROM_ASCII )
remove_definitions( -DQT_NO_CAST_TO_ASCII )
########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5LibKSieve")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5LibKSieveConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5LibKSieveConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5LibKSieveConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5LibKSieveConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5KSieveTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5LibKSieveTargets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/libksieve_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
else()
ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX LIBKSIEVE
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/libksieve_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5LibKSieveConfigVersion.cmake"
                        SOVERSION 5
)
endif()


add_subdirectory(src)
if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()
install( FILES libksieve.categories DESTINATION ${KDE_INSTALL_CONFDIR} )
