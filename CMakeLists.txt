project(kdepim)

cmake_minimum_required(VERSION 2.8.12)

set(LIBRARY_NAMELINK LIBRARY NAMELINK_SKIP)

# we need some parts of the ECM CMake helpers
find_package(ECM 5.19.0 REQUIRED NO_MODULE)
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

# KDEPIM_VERSION
# Version scheme: "x.y.z build".
#
# x is the version number.
# y is the major release number.
# z is the minor release number.
#
# "x.y.z" follow the kdelibs version kdepim is released with.
#
# If "z" is 0, the version is "x.y"
#
# KDEPIM_DEV_VERSION is empty for final versions.
# For development versions "build" is something like "pre", "alpha1", "alpha2", "beta1", "beta2", "rc1", "rc2".
#
# Examples in chronological order:
# 3.0, 3.0.1, 3.1 alpha1, 3.1 beta1, 3.1 beta2, 3.1 rc1, 3.1, 3.1.1, 3.2 pre, 3.2 alpha1

# Do NOT add quote
set(KDEPIM_DEV_VERSION )

# add an extra space
if(DEFINED KDEPIM_DEV_VERSION)
  set(KDEPIM_DEV_VERSION " ${KDEPIM_DEV_VERSION}")
endif()

set(KDEPIM_VERSION_NUMBER "5.2.0")
set(KDEPIM_VERSION "${KDEPIM_VERSION_NUMBER}${KDEPIM_DEV_VERSION}")


set(KDEPIMLIBS_LIB_VERSION "5.2.0")
set(KCONTACTS_LIB_VERSION "5.2.0")
set(KCALENDARCORE_LIB_VERSION "5.2.0")
set(IDENTITYMANAGEMENT_LIB_VERSION "5.2.0")
set(KLDAP_LIB_VERSION "5.2.0")
set(KMAILTRANSPORT_LIB_VERSION "5.2.0")
set(CALENDARUTILS_LIB_VERSION "5.2.0")
set(KHOLIDAYS_LIB_VERSION "5.2.0")
set(KTNEF_LIB_VERSION "5.2.0")
set(KIMAP_LIB_VERSION "5.2.0")
set(KMBOX_LIB_VERSION "5.2.0")
set(AKONADICALENDAR_LIB_VERSION "5.2.0")
set(SYNDICATION_LIB_VERSION "5.2.0")
set(GPGMEPP_LIB_VERSION "5.2.0")
set(KONTACTINTERFACE_LIB_VERSION "5.2.0")
set(AKONADIKALARM_LIB_VERSION "5.2.0")
set(KMIME_LIB_VERSION "5.2.0")
set(XMLRPCCLIENT_LIB_VERSION "5.2.0")
set(KBLOG_LIB_VERSION "5.2.0")
set(KPIMTEXTEDIT_LIB_VERSION "5.2.0")
set(AKONADI_VERSION "5.2.0")

set(KDEPIM_LIB_VERSION "${KDEPIM_VERSION_NUMBER}")
set(KDEPIM_LIB_SOVERSION "5")
set(AKONADINOTES_LIB_VERSION "5.2.0")

set(QT_REQUIRED_VERSION "5.5.0")
set(KF5_VERSION "5.19.0")
option(KDEPIM_ENTERPRISE_BUILD "Enable features specific to the enterprise branch, which are normally disabled. Also, it disables many components not needed for Kontact such as the Kolab client." FALSE)
option(QTWEBENGINE_EXPERIMENTAL_OPTION "Enable support for qtwebengine. False by default." FALSE)
find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Test WebKitWidgets Sql Concurrent UiTools OpenGL)
find_package(Qt5X11Extras NO_MODULE)
find_package(Grantlee5 "5.0" CONFIG REQUIRED)

if (QTWEBENGINE_EXPERIMENTAL_OPTION)
    set(QT_REQUIRED_MINIMUM_WEBENGINE_VERSION "5.6")
    find_package(Qt5 ${QT_REQUIRED_MINIMUM_WEBENGINE_VERSION} CONFIG REQUIRED WebEngine WebEngineWidgets)
endif()

# Find KF5 package
find_package(KF5KDELibs4Support ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Wallet ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5NewStuff ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KCMUtils ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5WebKit ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Archive ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5NotifyConfig ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Config ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Service ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5DBusAddons ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Auth ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5DocTools REQUIRED)
find_package(Phonon4Qt5 CONFIG REQUIRED)
find_package(KF5DNSSD ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5TextEditor ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Sonnet ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5GlobalAccel ${KF5_VERSION} CONFIG REQUIRED)
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
find_package(KF5AkonadiMime ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarUtils ${CALENDARUTILS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Holidays ${KHOLIDAYS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Tnef ${KTNEF_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5IMAP ${KIMAP_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Mbox ${KMBOX_LIB_VERSION} CONFIG REQUIRED)

find_package(KF5AkonadiCalendar ${AKONADICALENDAR_LIB_VERSION} CONFIG REQUIRED)

find_package(KF5Syndication ${SYNDICATION_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Gpgmepp ${GPGMEPP_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5KontactInterface ${KONTACTINTERFACE_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AlarmCalendar ${AKONADIKALARM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Mime ${KMIME_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5XmlRpcClient ${XMLRPCCLIENT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Blog ${KBLOG_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiNotes ${AKONADINOTES_LIB_VERSION} CONFIG REQUIRED)


set(LIBGRAVATAR_VERSION_LIB "5.2.0")
set(LIBMAILIMPORTER_VERSION_LIB "5.2.0")
set(MAILCOMMON_LIB_VERSION_LIB "5.2.0")
set(KDEPIM_APPS_LIB_VERSION_LIB "5.2.0")
set(MESSAGELIB_LIB_VERSION_LIB "5.2.0")
set(LIBKLEO_LIB_VERSION_LIB "5.2.0")
set(LIBGRANTLEETHEME_LIB_VERSION_LIB "5.2.0")
set(PIMCOMMON_LIB_VERSION_LIB "5.2.0")
set(LIBKDEPIM_LIB_VERSION_LIB "5.2.0")
set(LIBINCIDENCEEDITOR_LIB_VERSION_LIB "5.2.0")
set(CALENDARSUPPORT_LIB_VERSION_LIB "5.2.0")
set(EVENTVIEW_LIB_VERSION_LIB "5.2.0")
set(KDGANTT2_LIB_VERSION_LIB "5.2.0")
set(LIBKSIEVE_LIB_VERSION_LIB "5.2.0")
find_package(KF5Gravatar ${LIBGRAVATAR_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5MailImporter ${LIBMAILIMPORTER_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5MailCommon ${MAILCOMMON_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5KaddressbookGrantlee ${KDEPIM_APPS_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5MessageViewer ${MESSAGELIB_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5Libkleo ${LIBKLEO_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5GrantleeTheme ${LIBGRANTLEETHEME_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5PimCommon ${PIMCOMMON_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5Libkdepim ${LIBKDEPIM_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5IncidenceEditor ${LIBINCIDENCEEDITOR_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5MessageCore ${MESSAGELIB_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5MessageComposer ${KDEPIM_LIB_INSTALLED_VERSION} CONFIG REQUIRED)
find_package(KF5MessageList ${KDEPIM_LIB_INSTALLED_VERSION} CONFIG REQUIRED)
find_package(KF5TemplateParser ${KDEPIM_LIB_INSTALLED_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarSupport ${CALENDARSUPPORT_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5EventViews ${EVENTVIEW_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5KDGantt2 ${KDGANTT2_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5FollowupReminder ${KDEPIM_APPS_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5SendLater ${KDEPIM_LIB_INSTALLED_VERSION} CONFIG REQUIRED)
find_package(KF5ComposerEditorNG ${KDEPIM_LIB_INSTALLED_VERSION} CONFIG REQUIRED)
find_package(KF5KdepimDBusInterfaces ${KDEPIM_LIB_INSTALLED_VERSION} CONFIG REQUIRED)
find_package(KF5LibKSieve ${LIBKSIEVE_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(MailTransportDBusService CONFIG REQUIRED)


if (NOT APPLE)
    find_package(X11)
endif()

set(KDEPIM_HAVE_X11 ${X11_FOUND})
configure_file(config-kdepim.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdepim.h )


# Extra package
find_package(KF5GAPI "4.72.0" CONFIG)

configure_file(kdepim-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdepim-version.h @ONLY)

include_directories(${kdepim_SOURCE_DIR} ${kdepim_BINARY_DIR})

find_package(Xsltproc)
set_package_properties(Xsltproc PROPERTIES DESCRIPTION "XSLT processor from libxslt" TYPE REQUIRED PURPOSE "Required to generate D-Bus interfaces for all Akonadi resources.")

find_package(KF5AkonadiSearch "5.2.0" CONFIG REQUIRED)
set_package_properties(KF5AkonadiSearch PROPERTIES DESCRIPTION "The Akonadi Search libraries" URL "http://www.kde.org" TYPE REQUIRED PURPOSE "Provides search capabilities in KMail and Akonadi")

set(Prison_MIN_VERSION "1.2.1")
find_package(KF5Prison ${Prison_MIN_VERSION} CONFIG)
set_package_properties(Prison PROPERTIES DESCRIPTION "The Prison library" URL "http://projects.kde.org/prison" TYPE OPTIONAL PURPOSE "Needed to show mobile barcodes of your contacts")

if( KF5GAPI_FOUND )
  add_definitions( -DKDEPIM_STORAGESERVICE_GDRIVE )
endif()

if(KDEPIM_ENTERPRISE_BUILD)
  message(STATUS "Enterprise build is enabled.")
endif()

############### Desktop vs. Mobile options ##############

add_definitions(-DQT_USE_QSTRINGBUILDER)


find_package(Boost 1.34.0)
set_package_properties(Boost PROPERTIES DESCRIPTION "Boost C++ Libraries" URL "http://www.boost.org" TYPE REQUIRED PURPOSE "Boost is required for building most KDEPIM applications")

# config-enterprise.h is needed for both ENTERPRISE_BUILD and BUILD_EVERYTHING
configure_file(config-enterprise.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-enterprise.h )

add_subdirectory(kmail)
add_subdirectory(grantleeeditor)


add_subdirectory(korganizer)
add_subdirectory(korgac)
add_subdirectory(sieveeditor)
add_subdirectory(storageservicemanager)
add_subdirectory(agents)
add_subdirectory(akregator)
add_subdirectory(importwizard)
add_subdirectory(kaddressbook)
add_subdirectory(mboximporter)
add_subdirectory(knotes)
add_subdirectory(ktnef)
add_subdirectory(pimsettingexporter)
add_subdirectory(kalarm)
add_subdirectory(blogilo)


add_subdirectory(kontact)

add_subdirectory(akonadiconsole)
add_subdirectory(console)

add_subdirectory(plugins) # Must be after KMail

add_subdirectory(accountwizard)

# doc must be a subdir of kdepim or packagers will kill us
add_subdirectory(doc)

install( FILES kdepim.categories DESTINATION ${KDE_INSTALL_CONFDIR} )

feature_summary(WHAT ALL
                INCLUDE_QUIET_PACKAGES
                FATAL_ON_MISSING_REQUIRED_PACKAGES
)
