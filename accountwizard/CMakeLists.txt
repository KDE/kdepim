project(accountwizard)

if(${CMAKE_SOURCE_DIR} STREQUAL ${accountwizard_SOURCE_DIR})
    cmake_minimum_required(VERSION 3.1)

    set(LIBRARY_NAMELINK LIBRARY NAMELINK_SKIP)

    # we need some parts of the ECM CMake helpers
    set(KF5_VERSION "5.23.0")
    find_package(ECM ${KF5_VERSION} REQUIRED NO_MODULE)
    set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
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


    set(KDEPIM_VERSION_NUMBER "5.2.54")
    set(KDEPIMLIBS_LIB_VERSION "5.2.40")
    set(KCONTACTS_LIB_VERSION "5.2.40")
    set(IDENTITYMANAGEMENT_LIB_VERSION "5.2.40")
    set(KLDAP_LIB_VERSION "5.2.40")
    set(KMAILTRANSPORT_LIB_VERSION "5.2.40")
    set(KPIMTEXTEDIT_LIB_VERSION "5.2.40")
    set(AKONADI_VERSION "5.2.40")

    set(KDEPIM_LIB_VERSION "${KDEPIM_VERSION_NUMBER}")
    set(KDEPIM_LIB_SOVERSION "5")

    set(QT_REQUIRED_VERSION "5.6.0")
    option(KDEPIM_ENTERPRISE_BUILD "Enable features specific to the enterprise branch, which are normally disabled. Also, it disables many components not needed for Kontact such as the Kolab client." FALSE)
    find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Xml DBus Test)
    set(MAILCOMMON_LIB_VERSION_LIB "5.2.56")
    set(MESSAGELIB_LIB_VERSION_LIB "5.2.88")
    set(PIMCOMMON_LIB_VERSION_LIB "5.2.43")
    set(LIBKDEPIM_LIB_VERSION_LIB "5.2.40")

    # Find KF5 package
    find_package(KF5Wallet ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5KCMUtils ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5NotifyConfig ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5Config ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5Service ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5DBusAddons ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5DocTools ${KF5_VERSION} REQUIRED)
    find_package(KF5TextEditor ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5Codecs ${KF5_VERSION} CONFIG REQUIRED)

    # Find KdepimLibs Package
    find_package(KF5PimTextEdit ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5Akonadi ${AKONADI_VERSION} CONFIG REQUIRED)
    find_package(KF5Contacts ${KCONTACTS_LIB_VERSION} CONFIG REQUIRED)
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
    add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

endif()
add_definitions(-DQT_USE_QSTRINGBUILDER)

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
install( FILES accountwizard.categories DESTINATION ${KDE_INSTALL_CONFDIR} )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
