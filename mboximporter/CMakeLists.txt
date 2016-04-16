project(mboximporter)

if(${CMAKE_SOURCE_DIR} STREQUAL ${mboximporter_SOURCE_DIR})
    cmake_minimum_required(VERSION 2.8.12)
    find_package(ECM 5.20.0 REQUIRED NO_MODULE)
    set(CMAKE_MODULE_PATH ${ktnef_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})
    include(ECMInstallIcons)

    include(ECMOptionalAddSubdirectory)
    include(ECMSetupVersion)
    include(ECMAddTests)
    include(ECMMarkNonGuiExecutable)
    include(GenerateExportHeader)
    include(ECMGenerateHeaders)
    include(CMakePackageConfigHelpers)
    include(FeatureSummary)
    include(CheckFunctionExists)
    include(ECMGeneratePriFile)

    include(KDEInstallDirs)
    include(KDECMakeSettings)
    include(KDECompilerSettings NO_POLICY_SCOPE)
    include(ECMAddAppIcon)
    include(ECMQtDeclareLoggingCategory)


    set(KF5_VERSION "5.20.0")

    set(KDEPIM_VERSION_NUMBER "5.2.42")
    set(KDEPIM_VERSION "${KDEPIM_VERSION_NUMBER}${KDEPIM_DEV_VERSION}")


    set(KDEPIMLIBS_LIB_VERSION "5.2.40")
    set(KMAILTRANSPORT_LIB_VERSION "5.2.40")
    set(AKONADICALENDAR_LIB_VERSION "5.2.40")
    set(KPIMTEXTEDIT_LIB_VERSION "5.2.40")
    set(AKONADI_VERSION "5.2.40")

    set(KDEPIM_LIB_VERSION "${KDEPIM_VERSION_NUMBER}")
    set(KDEPIM_LIB_SOVERSION "5")
    set(AKONADINOTES_LIB_VERSION "5.2.40")

    set(QT_REQUIRED_VERSION "5.6.0")
    find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Test Sql Concurrent UiTools OpenGL)
    find_package(Qt5X11Extras NO_MODULE)
    set(LIBMAILIMPORTER_VERSION_LIB "5.2.40")
    set(MAILCOMMON_LIB_VERSION_LIB "5.2.40")

    # Find KF5 package
    find_package(KF5Config ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5Service ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5DBusAddons ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5DocTools REQUIRED)

    # Find KdepimLibs Package
    find_package(KF5Akonadi ${AKONADI_VERSION} CONFIG REQUIRED)

    find_package(KF5MailCommon ${MAILCOMMON_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5MailImporter ${LIBMAILIMPORTER_VERSION_LIB} CONFIG REQUIRED)
    include_directories(${mboximporter_SOURCE_DIR} ${mboximporter_BINARY_DIR})

endif()


add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


add_subdirectory(src)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
