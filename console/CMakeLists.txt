project(console)

if(${CMAKE_SOURCE_DIR} STREQUAL ${console_SOURCE_DIR})
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


    set(AKONADI_MIMELIB_VERSION "5.3.40")
    set(AKONADICALENDAR_LIB_VERSION "5.3.40")
    set(AKONADI_VERSION "5.3.43")
    set(LIBKDEPIM_LIB_VERSION_LIB "5.3.42")
    set(CALENDARSUPPORT_LIB_VERSION_LIB "5.3.41")
    set(KCALENDARCORE_LIB_VERSION "5.3.40")
    set(KCONTACTS_LIB_VERSION "5.3.43")

    set(KDEPIM_LIB_VERSION "${KDEPIM_VERSION_NUMBER}")
    set(KDEPIM_LIB_SOVERSION "5")

    set(QT_REQUIRED_VERSION "5.6.0")
    find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets)


    # Find KF5 package
    find_package(KF5DocTools ${KF5_VERSION} REQUIRED)

    # Find KdepimLibs Package
    find_package(KF5Akonadi ${AKONADI_VERSION} CONFIG REQUIRED)
    find_package(KF5Contacts ${KCONTACTS_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5CalendarCore ${KCALENDARCORE_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5AkonadiContact ${AKONADI_CONTACT_VERSION} CONFIG REQUIRED)
    find_package(KF5CalendarUtils ${CALENDARUTILS_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5AkonadiCalendar ${AKONADICALENDAR_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5Libkdepim ${LIBKDEPIM_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5CalendarSupport ${CALENDARSUPPORT_LIB_VERSION_LIB} CONFIG REQUIRED)

endif()
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
include_directories(${console_SOURCE_DIR} ${console_BINARY_DIR})
add_definitions(-DQT_USE_QSTRINGBUILDER)

configure_file(console-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/console-version.h @ONLY)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory(calendarjanitor)
add_subdirectory(konsolekalendar)

install( FILES console.categories DESTINATION ${KDE_INSTALL_CONFDIR} )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
