project( sieveeditor )
if(${CMAKE_SOURCE_DIR} STREQUAL ${sieveeditor_SOURCE_DIR})
    cmake_minimum_required(VERSION 3.1)
    set(KF5_VERSION "5.25.0")
    find_package(ECM ${KF5_VERSION} REQUIRED NO_MODULE)
    set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
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

    # Do NOT add quote
    set(KDEPIM_DEV_VERSION pre)

    # add an extra space
    if(DEFINED KDEPIM_DEV_VERSION)
        set(KDEPIM_DEV_VERSION " ${KDEPIM_DEV_VERSION}")
    endif()


    set(KDEPIM_VERSION_NUMBER "5.3.42")
    set(KDEPIM_VERSION "${KDEPIM_VERSION_NUMBER}${KDEPIM_DEV_VERSION}")

    set(KMAILTRANSPORT_LIB_VERSION "5.3.40")
    set(KPIMTEXTEDIT_LIB_VERSION "5.3.43")

    set(KDEPIM_LIB_VERSION "${KDEPIM_VERSION_NUMBER}")
    set(KDEPIM_LIB_SOVERSION "5")

    set(MESSAGELIB_LIB_VERSION_LIB "5.3.47")
    set(LIBKSIEVE_LIB_VERSION_LIB "5.3.42")
    set(PIMCOMMON_LIB_VERSION_LIB "5.3.41")

    set(QT_REQUIRED_VERSION "5.6.0")
    find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Network Test)

    # Find KF5 package
    find_package(KF5DBusAddons ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5DocTools ${KF5_VERSION} REQUIRED)
    find_package(KF5Crash ${KF5_VERSION} REQUIRED)
    find_package(KF5Bookmarks ${KF5_VERSION} REQUIRED)
    find_package(KF5IconThemes ${KF5_VERSION} REQUIRED)
    find_package(KF5KIO ${KF5_VERSION} REQUIRED)


    # Find KdepimLibs Package
    find_package(KF5PimTextEdit ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5MailTransport ${KMAILTRANSPORT_LIB_VERSION} CONFIG REQUIRED)

    find_package(KF5MessageViewer ${MESSAGELIB_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5PimCommon ${PIMCOMMON_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5LibKSieve ${LIBKSIEVE_LIB_VERSION_LIB} CONFIG REQUIRED)

    include_directories(${sieveeditor_SOURCE_DIR} ${sieveeditor_BINARY_DIR})

endif()
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_USE_QSTRINGBUILDER)
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()

install( FILES sieveeditor.categories DESTINATION ${KDE_INSTALL_CONFDIR} )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()

