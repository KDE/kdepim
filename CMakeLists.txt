project(kdepim)

cmake_minimum_required(VERSION 2.8.12)

# we need some parts of the ECM CMake helpers
find_package(ECM 1.3.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${kdepim_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})
include(ECMOptionalAddSubdirectory)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMAddTests)
include(ECMMarkNonGuiExecutable)

include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(CheckFunctionExists)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

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
set(KDEPIM_DEV_VERSION pre)

# add an extra space
if(DEFINED KDEPIM_DEV_VERSION)
  set(KDEPIM_DEV_VERSION " ${KDEPIM_DEV_VERSION}")
endif()

set(KDEPIM_VERSION "4.70${KDEPIM_DEV_VERSION}")


set(KDEPIMLIBS_LIB_VERSION "4.71.0")
set(KDEPIM_LIB_VERSION "4.70.0")
set(KDEPIM_LIB_SOVERSION "5")

set(QT_REQUIRED_VERSION "5.3.0")
set(KF5_VERSION "5.2.0")

option(KDEPIM_BUILD_EXAMPLES "Build the kdepim example applications." FALSE)
option(KDEPIM_BUILD_MOBILE "Build the mobile applications. Note that you have to enable KDEPIM_MOBILE_UI if you want to run these applications on a mobile device." TRUE)
option(KDEPIM_ENTERPRISE_BUILD "Enable features specific to the enterprise branch, which are normally disabled. Also, it disables many components not needed for Kontact such as the Kolab client." FALSE)
option(KDEPIM_MOBILE_UI "Build UI for mobile devices instead of for desktops" FALSE)
option(KDEPIM_ONLY_KLEO "Only build Kleopatra. This option will build only libkleo and kleopatra" FALSE)
option(KDEPIM_BUILD_STATIC "Build KDEPIM static." FALSE)
option(KDEPIM_BUILD_DESKTOP "Build Desktop Applications. Can be deactivated for mobile" TRUE)
option(KDEPIM_NO_WEBKIT "Do not use WebKit in the kdepim applications" FALSE)

find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets Test WebKitWidgets Sql Concurrent UiTools Declarative OpenGL)
find_package(Qt5X11Extras NO_MODULE)
#Need to use volatile_qt5_port branch from grantlee
find_package(Grantlee "1.0" CONFIG REQUIRED)

find_package(Qt5Designer NO_MODULE)
set_package_properties(Qt5Designer PROPERTIES
   PURPOSE "Required to build the Qt Designer plugins"
   TYPE OPTIONAL
)

# Find KF5 package
find_package(KF5KDELibs4Support ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Wallet ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5NewStuff ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KCMUtils ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5WebKit ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Archive ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5NotifyConfig ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Config ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KHtml ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Service ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5DBusAddons ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Auth ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5DocTools REQUIRED)
find_package(Phonon4Qt5 CONFIG REQUIRED)
find_package(KF5DNSSD ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5TextEditor ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Sonnet ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5GlobalAccel ${KF5_VERSION} CONFIG REQUIRED)

# Find KdepimLibs Package
find_package(KF5Akonadi CONFIG REQUIRED)
find_package(KF5Abc ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5PimUtils ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiContact ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5IdentityManagement ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Ldap ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5MailTransport ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiMime ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarUtils ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Holidays ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Tnef ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Imap ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Mbox ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiCalendar ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Syndication ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Gpgmepp ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5QGpgme ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5KontactInterface ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AlarmCalendar ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Mime ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Blog ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5XmlRpcClient ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)

if (NOT APPLE)
    find_package(X11)
endif()

set(KDEPIM_HAVE_X11 ${X11_FOUND})
configure_file(config-kdepim.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdepim.h )


# Extra package
find_package(KF5GAPI "4.71.0" CONFIG)

include(GenerateExportHeader)

configure_file(kdepim-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdepim-version.h @ONLY)

include_directories(${kdepim_SOURCE_DIR} ${kdepim_BINARY_DIR})

set(KF5AkonadiServer_MIN_VERSION "5.0.40")
find_package(KF5AkonadiServer ${KF5AkonadiServer_MIN_VERSION} CONFIG REQUIRED)

find_package(Xsltproc)
set_package_properties(Xsltproc PROPERTIES DESCRIPTION "XSLT processor from libxslt" TYPE REQUIRED PURPOSE "Required to generate D-Bus interfaces for all Akonadi resources.")

find_package(KF5Baloo "5.0.0" CONFIG REQUIRED)
set_package_properties(Baloo PROPERTIES DESCRIPTION "The Baloo libraries" URL "http://www.kde.org" TYPE REQUIRED PURPOSE "Baloo provides search capabilities in KMail and Akonadi")


find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES DESCRIPTION "The Zlib compression library" URL "http://www.zlib.net" TYPE REQUIRED)

set(Prison_MIN_VERSION "1.2.0")
find_package(KF5Prison ${Prison_MIN_VERSION} CONFIG)
set_package_properties(Prison PROPERTIES DESCRIPTION "The Prison library" URL "http://projects.kde.org/prison" TYPE OPTIONAL PURPOSE "Needed to show mobile barcodes of your contacts")

if( KF5GAPI_FOUND )
  add_definitions( -DKDEPIM_STORAGESERVICE_GDRIVE )
endif()

if(KDEPIM_ENTERPRISE_BUILD)
  message(STATUS "Enterprise build is enabled.")
endif()

if(KDEPIM_BUILD_STATIC)
  set(LIBRARY_TYPE STATIC)
else()
  set(LIBRARY_TYPE SHARED)
endif()


# if KDEPIM_ONLY_KLEO is defined, KDEPIM_BUILD_MOBILE and KDEPIM_MOBILE_UI are disabled.
if(KDEPIM_ONLY_KLEO)
 set(KDEPIM_BUILD_MOBILE FALSE)
 set(KDEPIM_MOBILE_UI FALSE)
 set(KDEPIM_DEFINITIONS "-DHAVE_CONFIG_H=1")
 message(STATUS "Only libkleo and Kleopatra will be built.")
endif()

if(KDEPIM_MOBILE_UI)
    # Build the mobile applications
    set(KDEPIM_BUILD_MOBILE TRUE)
endif()

############### Desktop vs. Mobile options ##############

if(KDEPIM_MOBILE_UI)
  add_definitions( -DKDEPIM_MOBILE_UI )
  if(NOT QT_QTDECLARATIVE_FOUND)
    message(FATAL_ERROR "The QtDeclarative (QML) module is required for building the mobile UI")
  endif()
else()
  if(NOT QT_QTDECLARATIVE_FOUND)
    message(STATUS "The Qt Declarative (QML) module was not found. The mobile applications will not be built.")
    set(KDEPIM_BUILD_MOBILE FALSE)
  endif()
endif()
add_definitions(-DQT_USE_QSTRINGBUILDER)


find_package(Boost 1.34.0)
set_package_properties(Boost PROPERTIES DESCRIPTION "Boost C++ Libraries" URL "http://www.boost.org" TYPE REQUIRED PURPOSE "Boost is required for building most KDEPIM applications")

# Kleopatra needs to know if the topological.hpp header exists (part of Boost_graph).
find_path(Boost_TOPOLOGICAL_SORT_DIR NAMES boost/graph/topological_sort.hpp PATHS ${Boost_INCLUDE_DIRS})
if(Boost_TOPOLOGICAL_SORT_DIR)
  message(STATUS "The Boost Topological_sort header was found. Building Kleopatra")
else()
  message(STATUS "The Boost Topological_sort header was NOT found. Kleopatra will not be built")
endif()


# config-enterprise.h is needed for both ENTERPRISE_BUILD and BUILD_EVERYTHING
configure_file(config-enterprise.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-enterprise.h )

add_subdirectory(noteshared)
add_subdirectory(akonadi_next)
add_subdirectory(libkdepim)
add_subdirectory(calendarsupport)
add_subdirectory(calendarviews)
add_subdirectory(incidenceeditor-ng)
add_subdirectory(libkdepimdbusinterfaces)
add_subdirectory(libkleo)
add_subdirectory(libkpgp)
add_subdirectory(libksieve)
add_subdirectory(kdgantt2)
add_subdirectory(icons)
add_subdirectory(composereditor-ng)
add_subdirectory(messagecore)
add_subdirectory(grantleetheme)
add_subdirectory(messagelist)
add_subdirectory(templateparser)
include (kleopatra/ConfigureChecks.cmake)

#if(QGpgme_FOUND)
  if(Boost_TOPOLOGICAL_SORT_DIR)
    add_subdirectory(kleopatra)
  endif()
#endif()

add_subdirectory(messageviewer)
add_subdirectory(messagecomposer)
add_subdirectory(pimcommon)
add_subdirectory(mailcommon) 
add_subdirectory(kmail)
add_subdirectory(grantleeeditor)
if(KDEPIM_BUILD_MOBILE)
  add_subdirectory(mobile)
endif()

if(KDEPIM_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

set(KMAIL_SUPPORTED TRUE)
add_definitions(-DKMAIL_SUPPORTED)

add_subdirectory(korganizer)
add_subdirectory(korgac)
add_subdirectory(sieveeditor)
add_subdirectory(storageservicemanager)
add_subdirectory(kaddressbookgrantlee)
if(KDEPIM_BUILD_DESKTOP)
  add_subdirectory(agents)
  add_subdirectory(akregator)
  add_subdirectory(importwizard)
  add_subdirectory(kaddressbook)
  add_subdirectory(kmailcvt)
  add_subdirectory(mboximporter)
  add_subdirectory(knotes)
  add_subdirectory(ksendemail)
  add_subdirectory(ktnef)
  add_subdirectory(mailimporter)
  add_subdirectory(pimsettingexporter)
  add_subdirectory(kalarm)
  add_subdirectory(blogilo)

  add_subdirectory(kjots)

  add_subdirectory(kontact)
endif()

add_subdirectory(akonadiconsole)
add_subdirectory(console)

add_subdirectory(plugins) # Must be after KMail

# doc must be a subdir of kdepim or packagers will kill us
add_subdirectory(doc)

feature_summary(WHAT ALL
                INCLUDE_QUIET_PACKAGES
                FATAL_ON_MISSING_REQUIRED_PACKAGES
)

