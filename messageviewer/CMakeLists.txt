cmake_minimum_required(VERSION 2.8.12)

project(MessageViewer)

if(${CMAKE_SOURCE_DIR} STREQUAL ${MessageViewer_SOURCE_DIR})
# ECM setup
find_package(ECM 5.16.0 CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${MessageViewer_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})
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
set(KDEPIMLIBS_LIB_VERSION "5.0.81")
set(KDEPIM_LIB_VERSION "5.0.90")
set(QT_REQUIRED_VERSION "5.4.0")
set(KMIME_LIB_VERSION "4.87.0")
set(KCONTACTS_LIB_VERSION "4.92.0")
set(KMBOX_LIB_VERSION "4.78.0")
set(AKONADINOTES_LIB_VERSION "4.78.0")
set(KPIMTEXTEDIT_LIB_VERSION "4.91.0")

find_package(KF5Akonadi ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Test UiTools WebKitWidgets)
find_package(KF5I18n ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KDELibs4Support ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Mime ${KMIME_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiMime ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5KaddressbookGrantlee ${KDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5GrantleeTheme ${KDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Libkleo ${KDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Libkdepim ${KDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5PimCommon ${KDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5MessageCore ${KDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5IncidenceEditorsng ${KDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5ItemViews ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Contacts ${KCONTACTS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Mbox ${KMBOX_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5IconThemes ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5WebKit ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiNotes ${AKONADINOTES_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5PimTextEdit ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Gravatar ${KDEPIM_LIB_VERSION} CONFIG REQUIRED)

find_package(Grantlee5 "5.0" CONFIG REQUIRED)
ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX MESSAGEVIEWER
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/messageviewer_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5MessageViewerConfigVersion.cmake"
                        SOVERSION 5
)

find_package(Gpgme REQUIRED)

########### Targets ###########

add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")
#verify
remove_definitions("-DQT_NO_URL_CAST_FROM_STRING")
remove_definitions( -DQT_NO_CAST_FROM_BYTEARRAY )

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5MessageViewer")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5MessageViewerConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5MessageViewerConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5MessageViewerConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5MessageViewerConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5MessageViewerTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5MessageViewerTargets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/messageviewer_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
else()
ecm_setup_version(${KDEPIM_LIB_VERSION} VARIABLE_PREFIX MESSAGEVIEWER
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/messageviewer_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5MessageViewerConfigVersion.cmake"
                        SOVERSION 5
)
endif()

add_subdirectory(src)
if (BUILD_TESTING)
   add_subdirectory(autotests)
   add_subdirectory(tests)
endif()

