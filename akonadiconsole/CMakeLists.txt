project(akonadiconsole)

if(${CMAKE_SOURCE_DIR} STREQUAL ${akonadiconsole_SOURCE_DIR})
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
    set(KDEPIM_DEV_VERSION pre)

    # add an extra space
    if(DEFINED KDEPIM_DEV_VERSION)
        set(KDEPIM_DEV_VERSION " ${KDEPIM_DEV_VERSION}")
    endif()


    set(KDEPIM_VERSION_NUMBER "5.3.40")
    set(KDEPIM_VERSION "${KDEPIM_VERSION_NUMBER}${KDEPIM_DEV_VERSION}")


    set(AKONADI_MIMELIB_VERSION "5.3.40")
    set(AKONADI_CONTACT_VERSION "5.3.40")
    set(CALENDARSUPPORT_LIB_VERSION_LIB "5.3.41")
    set(KPIMTEXTEDIT_LIB_VERSION "5.3.40")
    set(AKONADI_VERSION "5.3.40")

    set(KDEPIM_LIB_VERSION "${KDEPIM_VERSION_NUMBER}")
    set(KDEPIM_LIB_SOVERSION "5")


    set(QT_REQUIRED_VERSION "5.6.0")
    find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets DBus Sql)
    set(LIBMAILIMPORTER_VERSION_LIB "5.3.40")
    set(MESSAGELIB_LIB_VERSION_LIB "5.3.44")
    set(LIBKLEO_LIB_VERSION_LIB "5.3.40")
    set(LIBKDEPIM_LIB_VERSION_LIB "5.3.40")
    set(KCALENDARCORE_LIB_VERSION "5.3.40")
    set(KCONTACTS_LIB_VERSION "5.3.42")
    set(KMIME_LIB_VERSION "5.3.40")

    # Find KF5 package
    find_package(KF5Completion ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5Config ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5ConfigWidgets ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5DBusAddons ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5DocTools ${KF5_VERSION} REQUIRED)
    find_package(KF5I18n ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5ItemModels ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5KDELibs4Support ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5TextWidgets ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5WidgetsAddons ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5XmlGui ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5Crash ${KF5_VERSION} REQUIRED)

    # Find KdepimLibs Package
    find_package(KF5PimTextEdit ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5Akonadi ${AKONADI_VERSION} CONFIG REQUIRED)
    find_package(KF5Contacts ${KCONTACTS_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5CalendarCore ${KCALENDARCORE_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5AkonadiContact ${AKONADI_CONTACT_VERSION} CONFIG REQUIRED)
    find_package(KF5CalendarSupport ${CALENDARSUPPORT_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5AkonadiMime ${AKONADI_MIMELIB_VERSION} CONFIG REQUIRED)


    find_package(KF5MailImporter ${LIBMAILIMPORTER_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5MessageViewer ${MESSAGELIB_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5Mime ${KMIME_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5Libkdepim ${LIBKDEPIM_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5Libkleo ${LIBKLEO_LIB_VERSION_LIB} CONFIG REQUIRED)

    include_directories(${akonadiconsole_SOURCE_DIR} ${akonadiconsole_BINARY_DIR})
    add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
endif()

add_definitions(-DQT_USE_QSTRINGBUILDER)
install( FILES akonadiconsole.categories DESTINATION ${KDE_INSTALL_CONFDIR} )

add_subdirectory(src)
if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
