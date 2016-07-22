project(grantleeditor)

if(${CMAKE_SOURCE_DIR} STREQUAL ${grantleeditor_SOURCE_DIR})
    cmake_minimum_required(VERSION 3.1)
    set(KF5_VERSION "5.23.0")
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
    set(KDEPIM_DEV_VERSION beta1)

    # add an extra space
    if(DEFINED KDEPIM_DEV_VERSION)
        set(KDEPIM_DEV_VERSION " ${KDEPIM_DEV_VERSION}")
    endif()


    set(KDEPIM_VERSION_NUMBER "5.2.90")
    set(KDEPIM_VERSION "${KDEPIM_VERSION_NUMBER}${KDEPIM_DEV_VERSION}")

    set(MESSAGELIB_LIB_VERSION_LIB "5.2.91")
    set(AKONADI_MIMELIB_VERSION "5.2.80")
    set(KPIMTEXTEDIT_LIB_VERSION "5.2.80")
    set(KDEPIM_LIB_VERSION "${KDEPIM_VERSION_NUMBER}")
    set(KDEPIM_LIB_SOVERSION "5")
    set(KDEPIM_APPS_LIB_VERSION_LIB "5.2.80")
    set(LIBGRANTLEETHEME_LIB_VERSION_LIB "5.2.80")
    set(LIBKLEO_LIB_VERSION_LIB "5.2.80")
    set(QT_REQUIRED_VERSION "5.6.0")
    find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets WebEngine WebEngineWidgets)

    set(PIMCOMMON_LIB_VERSION_LIB "5.2.81")
    set(LIBKDEPIM_LIB_VERSION_LIB "5.2.80")
    # Find KF5 package
    find_package(KF5Crash ${KF5_VERSION} REQUIRED)
    find_package(KF5DBusAddons ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5DocTools ${KF5_VERSION} REQUIRED)
    find_package(KF5XmlGui ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5TextEditor ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5Wallet ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5NewStuff ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5PimCommon ${PIMCOMMON_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5MessageViewer ${MESSAGELIB_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5GrantleeTheme ${LIBGRANTLEETHEME_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5AkonadiMime ${AKONADI_MIMELIB_VERSION} CONFIG REQUIRED)
    find_package(KF5Libkleo ${LIBKLEO_LIB_VERSION_LIB} CONFIG REQUIRED)

    # Find KdepimLibs Package
    find_package(KF5KaddressbookGrantlee ${KDEPIM_APPS_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5PimTextEdit ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5Libkdepim ${LIBKDEPIM_LIB_VERSION_LIB} CONFIG REQUIRED)
    add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
endif()

add_definitions(-DQT_USE_QSTRINGBUILDER)
include_directories(${grantleeditor_SOURCE_DIR} ${grantleeditor_BINARY_DIR})

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

configure_file(grantleeeditor-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/grantleeeditor-version.h @ONLY)

add_subdirectory(headerthemeeditor)
add_subdirectory(contactthemeeditor)
add_subdirectory(grantleethemeeditor)
add_subdirectory(contactprintthemeeditor)

install( FILES grantleeditor.categories DESTINATION ${KDE_INSTALL_CONFDIR} )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
