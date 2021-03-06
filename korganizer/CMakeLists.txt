project(korganizer)

if(${CMAKE_SOURCE_DIR} STREQUAL ${korganizer_SOURCE_DIR})
    cmake_minimum_required(VERSION 3.1)
    if (POLICY CMP0063)
        cmake_policy(SET CMP0063 NEW)
    endif()
    set(KF5_VERSION "5.25.0")
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

    # Do NOT add quote
    set(KDEPIM_DEV_VERSION pre)

    # add an extra space
    if(DEFINED KDEPIM_DEV_VERSION)
        set(KDEPIM_DEV_VERSION " ${KDEPIM_DEV_VERSION}")
    endif()

    set(KDEPIM_VERSION_NUMBER "5.3.42")
    set(KDEPIM_VERSION "${KDEPIM_VERSION_NUMBER}${KDEPIM_DEV_VERSION}")


    set(AKONADI_MIMELIB_VERSION "5.3.40")
    set(AKONADI_CONTACT_VERSION "5.3.40")
    set(KCALENDARCORE_LIB_VERSION "5.3.40")
    set(IDENTITYMANAGEMENT_LIB_VERSION "5.3.41")
    set(KLDAP_LIB_VERSION "5.3.40")
    set(KMAILTRANSPORT_LIB_VERSION "5.3.40")
    set(CALENDARUTILS_LIB_VERSION "5.3.40")
    set(KHOLIDAYS_LIB_VERSION "5.3.40")
    set(AKONADICALENDAR_LIB_VERSION "5.3.40")
    set(KONTACTINTERFACE_LIB_VERSION "5.3.40")
    set(KMIME_LIB_VERSION "5.3.40")
    set(KPIMTEXTEDIT_LIB_VERSION "5.3.43")
    set(AKONADI_VERSION "5.3.43")

    set(KDEPIM_LIB_VERSION "${KDEPIM_VERSION_NUMBER}")
    set(KDEPIM_LIB_SOVERSION "5")
    set(AKONADINOTES_LIB_VERSION "5.3.40")

    set(QT_REQUIRED_VERSION "5.6.0")
    option(KDEPIM_ENTERPRISE_BUILD "Enable features specific to the enterprise branch, which are normally disabled. Also, it disables many components not needed for Kontact such as the Kolab client." FALSE)
    find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Test UiTools)
    find_package(Qt5X11Extras NO_MODULE)
    set(MAILCOMMON_LIB_VERSION_LIB "5.3.42")
    set(KDEPIM_APPS_LIB_VERSION_LIB "5.3.40")
    set(PIMCOMMON_LIB_VERSION_LIB "5.3.41")
    set(LIBKDEPIM_LIB_VERSION_LIB "5.3.42")
    set(LIBINCIDENCEEDITOR_LIB_VERSION_LIB "5.3.41")
    set(CALENDARSUPPORT_LIB_VERSION_LIB "5.3.41")
    set(EVENTVIEW_LIB_VERSION_LIB "5.3.40")
    set(KCONTACTS_LIB_VERSION "5.3.43")

    find_package(KF5AkonadiSearch "5.3.40" CONFIG REQUIRED)
    set_package_properties(KF5AkonadiSearch PROPERTIES DESCRIPTION "The Akonadi Search libraries" URL "http://www.kde.org" TYPE REQUIRED PURPOSE "Provides search capabilities in KMail and Akonadi")

    # Find KF5 package
    find_package(KF5NewStuff ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5KCMUtils ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5Config ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5Service ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5DBusAddons ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5DocTools ${KF5_VERSION} REQUIRED)
    find_package(KF5Codecs ${KF5_VERSION} CONFIG REQUIRED)
    find_package(KF5Crash ${KF5_VERSION} REQUIRED)

    # Find KdepimLibs Package
    find_package(KF5PimTextEdit ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5Akonadi ${AKONADI_VERSION} CONFIG REQUIRED)
    find_package(KF5Contacts ${KCONTACTS_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5CalendarCore ${KCALENDARCORE_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5AkonadiContact ${AKONADI_CONTACT_VERSION} CONFIG REQUIRED)
    find_package(KF5IdentityManagement ${IDENTITYMANAGEMENT_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5MailTransport ${KMAILTRANSPORT_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5AkonadiMime ${AKONADI_MIMELIB_VERSION} CONFIG REQUIRED)
    find_package(KF5CalendarUtils ${CALENDARUTILS_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5Holidays ${KHOLIDAYS_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5Ldap ${KLDAP_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5AkonadiCalendar ${AKONADICALENDAR_LIB_VERSION} CONFIG REQUIRED)
    find_package(Phonon4Qt5 CONFIG REQUIRED)

    find_package(KF5KontactInterface ${KONTACTINTERFACE_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5Mime ${KMIME_LIB_VERSION} CONFIG REQUIRED)
    find_package(KF5AkonadiNotes ${AKONADINOTES_LIB_VERSION} CONFIG REQUIRED)

    find_package(KF5KdepimDBusInterfaces ${KDEPIM_APPS_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5MailCommon ${MAILCOMMON_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5PimCommon ${PIMCOMMON_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5Libkdepim ${LIBKDEPIM_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5IncidenceEditor ${LIBINCIDENCEEDITOR_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5CalendarSupport ${CALENDARSUPPORT_LIB_VERSION_LIB} CONFIG REQUIRED)
    find_package(KF5EventViews ${EVENTVIEW_LIB_VERSION_LIB} CONFIG REQUIRED)
    if (NOT APPLE)
        find_package(X11)
    endif()
    find_package(Boost 1.34.0)
    set_package_properties(Boost PROPERTIES DESCRIPTION "Boost C++ Libraries" URL "http://www.boost.org" TYPE REQUIRED PURPOSE "Boost is required for building most KDEPIM applications")

    set(KDEPIM_HAVE_X11 ${X11_FOUND})
endif()
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_USE_QSTRINGBUILDER)
configure_file(config-korganizer.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-korganizer.h )
configure_file(korgac/config-enterprise.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-enterprise.h )
include_directories(${korganizer_SOURCE_DIR} ${korganizer_BINARY_DIR} ${korgac_SOURCE_DIR} ${korgac_BINARY_DIR})
configure_file(korganizer-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/korganizer-version.h @ONLY)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory(src)
add_subdirectory(korgac)


install( FILES korganizer.categories DESTINATION ${KDE_INSTALL_CONFDIR} )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
