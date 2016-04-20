project(accountwizard)

if(${CMAKE_SOURCE_DIR} STREQUAL ${accountwizard_SOURCE_DIR})
    cmake_minimum_required(VERSION 2.8.12)

    set(LIBRARY_NAMELINK LIBRARY NAMELINK_SKIP)

    # we need some parts of the ECM CMake helpers
    find_package(ECM 5.20.0 REQUIRED NO_MODULE)
    set(CMAKE_MODULE_PATH ${kdepim_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})
    include(ECMOptionalAddSubdirectory)
    include(ECMInstallIcons)
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


    set(KDEPIM_VERSION_NUMBER "5.2.43")
    set(KDEPIMLIBS_LIB_VERSION "5.2.40")
    set(KCONTACTS_LIB_VERSION "5.2.40")
    set(KCALENDARCORE_LIB_VERSION "5.2.40")
    set(IDENTITYMANAGEMENT_LIB_VERSION "5.2.40")
    set(KLDAP_LIB_VERSION "5.2.40")
    set(KMAILTRANSPORT_LIB_VERSION "5.2.40")
    set(CALENDARUTILS_LIB_VERSION "5.2.40")
    set(KHOLIDAYS_LIB_VERSION "5.2.40")
    set(KTNEF_LIB_VERSION "5.2.40")
    set(KIMAP_LIB_VERSION "5.2.40")
    set(KMBOX_LIB_VERSION "5.2.40")
    set(AKONADICALENDAR_LIB_VERSION "5.2.40")
    set(GPGMEPP_LIB_VERSION "5.2.40")
    set(KONTACTINTERFACE_LIB_VERSION "5.2.40")
    set(AKONADIKALARM_LIB_VERSION "5.2.40")
    set(KMIME_LIB_VERSION "5.2.40")
    set(XMLRPCCLIENT_LIB_VERSION "5.2.40")
    set(KBLOG_LIB_VERSION "5.2.40")
    set(KPIMTEXTEDIT_LIB_VERSION "5.2.40")
    set(AKONADI_VERSION "5.2.40")

    set(KDEPIM_LIB_VERSION "${KDEPIM_VERSION_NUMBER}")
    set(KDEPIM_LIB_SOVERSION "5")
    set(AKONADINOTES_LIB_VERSION "5.2.40")

    set(QT_REQUIRED_VERSION "5.6.0")
    set(KF5_VERSION "5.20.0")
    option(KDEPIM_ENTERPRISE_BUILD "Enable features specific to the enterprise branch, which are normally disabled. Also, it disables many components not needed for Kontact such as the Kolab client." FALSE)
    find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Test UiTools)
    find_package(Qt5X11Extras NO_MODULE)
    find_package(Grantlee5 "5.0" CONFIG REQUIRED)
    set(LIBGRAVATAR_VERSION_LIB "5.2.40")
    set(LIBMAILIMPORTER_VERSION_LIB "5.2.40")
    set(MAILCOMMON_LIB_VERSION_LIB "5.2.40")
    set(KDEPIM_APPS_LIB_VERSION_LIB "5.2.42")
    set(MESSAGELIB_LIB_VERSION_LIB "5.2.57")
    set(LIBKLEO_LIB_VERSION_LIB "5.2.40")
    set(LIBGRANTLEETHEME_LIB_VERSION_LIB "5.2.40")
    set(PIMCOMMON_LIB_VERSION_LIB "5.2.40")
    set(LIBKDEPIM_LIB_VERSION_LIB "5.2.40")

    # Find KF5 package
    find_package(KF5Wallet ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5NewStuff ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5KCMUtils ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5NotifyConfig ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5Config ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5Service ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5DBusAddons ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5DocTools REQUIRED)
    find_package(KF5TextEditor ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5Codecs ${KF5_VERSION} CONFIG REQUIRED)

    # Find KdepimLibs Package
    find_package(KF5PimTextEdit ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5Akonadi ${AKONADI_VERSION} CONFIG REQUIRED)
    find_package(KF5Contacts ${KCONTACTS_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5CalendarCore ${KCALENDARCORE_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5AkonadiContact ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5IdentityManagement ${IDENTITYMANAGEMENT_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5Ldap ${KLDAP_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5MailTransport ${KMAILTRANSPORT_LIB_VERSION} CONFIG REQUIRED)


    find_package(KF5MailCommon ${MAILCOMMON_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5MessageViewer ${MESSAGELIB_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5PimCommon ${PIMCOMMON_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5Libkdepim ${LIBKDEPIM_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(MailTransportDBusService CONFIG REQUIRED)


    include_directories(${accountwizard_SOURCE_DIR} ${accountwizard_BINARY_DIR})

endif()

# Qt5 package
find_package(Qt5UiTools ${QT_REQUIRED_VERSION} CONFIG REQUIRED)

find_package(KF5Kross ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5NewStuff ${KF5_VERSION} CONFIG REQUIRED)

set( SHARED_MIME_INFO_MINIMUM_VERSION "0.40" )
find_package( SharedMimeInfo REQUIRED )

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

# allow to disable GHNS support on resource-constrained systems
option( ACCOUNTWIZARD_NO_GHNS "Disable GHNS support in the account wizard" FALSE )

option(KDEPIM_ENTERPRISE_BUILD "Enable features specific to the enterprise branch, which are normally disabled. Also, it disables many components not needed for Kontact such as the Kolab client." FALSE)

# config-enterprise.h is needed for both ENTERPRISE_BUILD and BUILD_EVERYTHING
configure_file(src/config-enterprise.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config-enterprise.h )

if ( ACCOUNTWIZARD_NO_GHNS )
    add_definitions( -DACCOUNTWIZARD_NO_GHNS )
endif ()



add_subdirectory(src)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
