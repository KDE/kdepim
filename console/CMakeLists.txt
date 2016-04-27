project(console)

if(${CMAKE_SOURCE_DIR} STREQUAL ${console_SOURCE_DIR})
    cmake_minimum_required(VERSION 2.8.12)
    find_package(ECM 5.20.0 REQUIRED NO_MODULE)
    set(CMAKE_MODULE_PATH ${console_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})
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

    set(KDEPIM_VERSION_NUMBER "5.2.45")
    set(KDEPIM_VERSION "${KDEPIM_VERSION_NUMBER}${KDEPIM_DEV_VERSION}")


    set(KDEPIMLIBS_LIB_VERSION "5.2.40")
    set(AKONADICALENDAR_LIB_VERSION "5.2.40")
    set(AKONADI_VERSION "5.2.40")
    set(LIBKDEPIM_LIB_VERSION_LIB "5.2.40")
    set(CALENDARSUPPORT_LIB_VERSION_LIB "5.2.40")
    set(KCALENDARCORE_LIB_VERSION "5.2.40")
    set(KCONTACTS_LIB_VERSION "5.2.40")

    set(KDEPIM_LIB_VERSION "${KDEPIM_VERSION_NUMBER}")
    set(KDEPIM_LIB_SOVERSION "5")

    set(QT_REQUIRED_VERSION "5.6.0")
    find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Test)
    find_package(Qt5X11Extras NO_MODULE)


    # Find KF5 package
    find_package(KF5DocTools REQUIRED)

    # Find KdepimLibs Package
    find_package(KF5Akonadi ${AKONADI_VERSION} CONFIG REQUIRED)
    find_package(KF5Contacts ${KCONTACTS_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5CalendarCore ${KCALENDARCORE_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5AkonadiContact ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5CalendarUtils ${CALENDARUTILS_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5AkonadiCalendar ${AKONADICALENDAR_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5Libkdepim ${LIBKDEPIM_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5CalendarSupport ${CALENDARSUPPORT_LIB_VERSION_LIB} CONFIG REQUIRED)
    add_definitions(-DQT_USE_QSTRINGBUILDER)

endif()
include_directories(${console_SOURCE_DIR} ${console_BINARY_DIR})

configure_file(console-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/console-version.h @ONLY)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory(calendarjanitor)
add_subdirectory(konsolekalendar)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
