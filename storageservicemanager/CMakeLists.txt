project( storageservicemanager )

if(${CMAKE_SOURCE_DIR} STREQUAL ${storageservicemanager_SOURCE_DIR})
    cmake_minimum_required(VERSION 2.8.12)

    if (POLICY CMP0063)
        cmake_policy(SET CMP0063 NEW)
    endif()

    find_package(ECM 5.22.0 REQUIRED NO_MODULE)
    set(CMAKE_MODULE_PATH ${storageservicemanager_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})
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


    set(KF5_VERSION "5.22.0")

    set(KDEPIM_VERSION_NUMBER "5.2.47")
    set(KDEPIM_VERSION "${KDEPIM_VERSION_NUMBER}${KDEPIM_DEV_VERSION}")

    set(KDEPIMLIBS_LIB_VERSION "5.2.40")
    set(KPIMTEXTEDIT_LIB_VERSION "5.2.40")

    set(KDEPIM_LIB_VERSION "${KDEPIM_VERSION_NUMBER}")
    set(KDEPIM_LIB_SOVERSION "5")


    set(QT_REQUIRED_VERSION "5.6.0")
    find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Gui Network Test)
    set(PIMCOMMON_LIB_VERSION_LIB "5.2.40")
    set(LIBKDEPIM_LIB_VERSION_LIB "5.2.40")

    # Find KF5 package
    find_package(KF5DBusAddons ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5I18n ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5XmlGui ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5NotifyConfig ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5DocTools ${KF5_VERSION} REQUIRED)

    # Find KdepimLibs Package
    find_package(KF5PimTextEdit ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)


    find_package(KF5Libkdepim ${LIBKDEPIM_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5PimCommon ${PIMCOMMON_LIB_VERSION_LIB} CONFIG REQUIRED)

    include_directories(${storageservicemanager_SOURCE_DIR} ${storageservicemanager_BINARY_DIR})

endif()

add_definitions(-DQT_USE_QSTRINGBUILDER)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory(src)


install( FILES storageservicemanager.categories DESTINATION ${KDE_INSTALL_CONFDIR} )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
