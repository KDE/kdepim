project(kdepim)

cmake_minimum_required(VERSION 2.8.12)

# we need some parts of the ECM CMake helpers
find_package (ECM 0.0.12 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${kdepim_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})
include(ECMOptionalAddSubdirectory)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMAddTests)
include(ECMMarkNonGuiExecutable)
include(ECMGenerateHeaders)

include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(WriteBasicConfigVersionFile)
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

set(KDEPIM_VERSION "4.98${KDEPIM_DEV_VERSION}")


set(QT_REQUIRED_VERSION "5.2.0")
set(KF5_VERSION "4.98.0")

option(KDEPIM_BUILD_EXAMPLES "Build the kdepim example applications." FALSE)
option(KDEPIM_BUILD_MOBILE "Build the mobile applications. Note that you have to enable KDEPIM_MOBILE_UI if you want to run these applications on a mobile device." TRUE)
option(KDEPIM_ENTERPRISE_BUILD "Enable features specific to the enterprise branch, which are normally disabled. Also, it disables many components not needed for Kontact such as the Kolab client." FALSE)
option(KDEPIM_MOBILE_UI "Build UI for mobile devices instead of for desktops" FALSE)
option(KDEPIM_ONLY_KLEO "Only build Kleopatra. This option will build only libkleo and kleopatra" FALSE)
option(KDEPIM_BUILD_STATIC "Build KDEPIM static." FALSE)
option(KDEPIM_BUILD_DESKTOP "Build Desktop Applications. Can be deactivated for mobile" TRUE)
option(KDEPIM_NO_WEBKIT "Do not use WebKit in the kdepim applications" FALSE)


find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Widgets Test WebKitWidgets Sql Concurrent)
#Need to use volatile_qt5_port branch from grantlee
find_package(Grantlee "1.0" CONFIG REQUIRED)

find_package(KF5Akonadi CONFIG REQUIRED)
find_package(KF5KDELibs4Support ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Wallet ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5NewStuff ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Abc ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5PimUtils ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiContact ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5PimIdentities ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KLdap ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5MailTransport ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KCMUtils ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiKMime ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarUtils ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Holidays ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KTnef ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Imap ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Mbox ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiCalendar ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5WebKit ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Archive ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Imap ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5NotifyConfig ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Config ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Syndication ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KHtml ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Gpgmepp ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5QGpgme ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KontactInterface ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KAlarmCal ${KF5_VERSION} CONFIG REQUIRED)
include(GenerateExportHeader)

configure_file(kdepim-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdepim-version.h @ONLY)

include_directories(${kdepim_SOURCE_DIR} ${kdepim_BINARY_DIR})

set(AkonadiServer_MIN_VERSION "1.70.41")
find_package(Akonadi ${AkonadiServer_MIN_VERSION} CONFIG REQUIRED)

find_package(Xsltproc)
set_package_properties(Xsltproc PROPERTIES DESCRIPTION "XSLT processor from libxslt" TYPE REQUIRED PURPOSE "Required to generate D-Bus interfaces for all Akonadi resources.")

find_package(Baloo "5.0.0" CONFIG REQUIRED)
set_package_properties(Baloo PROPERTIES DESCRIPTION "The Baloo libraries" URL "http://www.kde.org" TYPE REQUIRED PURPOSE "Baloo provides search capabilities in KMail and Akonadi")


find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES DESCRIPTION "The Zlib compression library" URL "http://www.zlib.net" TYPE REQUIRED)


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

#add_subdirectory(noteshared)
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

  # These targets will always be built before anything else.

  #if(QGpgme_FOUND)
    if(Boost_TOPOLOGICAL_SORT_DIR)
      add_subdirectory(kleopatra)
    endif()
  #endif()

  # The following components depend on QGpgME.
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

  # If kmail is compiled, KMAIL_SUPPORTED is true (used in several places)
  if(BUILD_kmail)
    set(KMAIL_SUPPORTED TRUE)
    add_definitions(-DKMAIL_SUPPORTED)
  endif()

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
    #add_subdirectory(knotes)
    add_subdirectory(ksendemail)
    add_subdirectory(ktnef)
    add_subdirectory(mailimporter)
    add_subdirectory(pimsettingexporter)
    #add_subdirectory(kalarm)
    add_subdirectory(blogilo)

    add_subdirectory(kjots)

    add_subdirectory(kontact) # must be the last one.
  endif()

  add_subdirectory(akonadiconsole)
  add_subdirectory(console)

  add_subdirectory(plugins) # Must be after KMail

feature_summary(WHAT ALL
                INCLUDE_QUIET_PACKAGES
                FATAL_ON_MISSING_REQUIRED_PACKAGES
)

if (false)
# where to look first for cmake modules. This line must be the first one or cmake will use the system's FindFoo.cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

############### Build Options ###############

option(KDEPIM_BUILD_EXAMPLES "Build the kdepim example applications." FALSE)
option(KDEPIM_BUILD_MOBILE "Build the mobile applications. Note that you have to enable KDEPIM_MOBILE_UI if you want to run these applications on a mobile device." TRUE)
option(KDEPIM_ENTERPRISE_BUILD "Enable features specific to the enterprise branch, which are normally disabled. Also, it disables many components not needed for Kontact such as the Kolab client." FALSE)
option(KDEPIM_MOBILE_UI "Build UI for mobile devices instead of for desktops" FALSE)
option(KDEPIM_ONLY_KLEO "Only build Kleopatra. This option will build only libkleo and kleopatra" FALSE)
option(KDEPIM_BUILD_STATIC "Build KDEPIM static." FALSE)
option(KDEPIM_BUILD_DESKTOP "Build Desktop Applications. Can be deactivated for mobile" TRUE)
option(KDEPIM_NO_WEBKIT "Do not use WebKit in the kdepim applications" FALSE)

if(KDEPIM_BUILD_STATIC)
  set(LIBRARY_TYPE STATIC)
else()
  set(LIBRARY_TYPE SHARED)
endif()

add_definitions(-DQT_USE_QSTRINGBUILDER)
if(KDEPIM_NO_WEBKIT)
  add_definitions(-DKDEPIM_NO_WEBKIT)
endif()

if(KDEPIM_ENTERPRISE_BUILD)
  message(STATUS "Enterprise build is enabled.")
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

# config-enterprise.h is needed for both ENTERPRISE_BUILD and BUILD_EVERYTHING
configure_file(config-enterprise.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-enterprise.h )

############### generate kdepim-version.h ###############
# Support for the GIT revision number in kdepim-version.h
if(EXISTS "${kdepim_SOURCE_DIR}/.git")
  find_package(Git)
  if(GIT_FOUND)
    message(STATUS "Found git: ${GIT_EXECUTABLE}")
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                    WORKING_DIRECTORY ${kdepim_SOURCE_DIR}
                    OUTPUT_VARIABLE kdepim_git_revision)
    string(REGEX REPLACE "\n" "" kdepim_git_revision "${kdepim_git_revision}")
    set(kdepim_git_revision "git-${kdepim_git_revision}")

    execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --oneline --format=%ci
                    WORKING_DIRECTORY ${kdepim_SOURCE_DIR}
                    OUTPUT_VARIABLE kdepim_git_last_change)
    string(REGEX REPLACE " [-0-9:+ ]*\n" "" kdepim_git_last_change "${kdepim_git_last_change}")
  endif()
endif()

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

set(KDEPIM_VERSION "4.14${KDEPIM_DEV_VERSION}")

configure_file(kdepim-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdepim-version.h @ONLY)

set(CMAKE_AUTOMOC ON)

############### search packages used by KDE ###############

find_package(KDE4 4.12.0 REQUIRED)
include(KDE4Defaults)

find_package(KdepimLibs 4.13.45)
set_package_properties(KdepimLibs PROPERTIES DESCRIPTION "The KDEPIM libraries" URL "http://www.kde.org" TYPE REQUIRED)

############### Load the CTest options ###############

# CMake is irritating and doesn't allow setting the tests timeout globally.
# Let's work around this. The global timeout is now 2 minutes.
set(_DartConfigFile "${CMAKE_BINARY_DIR}/DartConfiguration.tcl")
if(EXISTS ${_DartConfigFile})
  set(DartTestingTimeout "120")
  file(READ ${_DartConfigFile} _DartConfigFile_content)
  string(REGEX REPLACE "TimeOut: 1500" "TimeOut: ${DartTestingTimeout}" _DartConfigFile_content ${_DartConfigFile_content})
  file(WRITE ${_DartConfigFile} ${_DartConfigFile_content})
endif()

# CTestCustom.cmake has to be in the CTEST_BINARY_DIR.
# in the KDE build system, this is the same as CMAKE_BINARY_DIR.
configure_file(${CMAKE_SOURCE_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR}/CTestCustom.cmake)

############### search Boost ###############

find_package(Boost 1.34.0)
set_package_properties(Boost PROPERTIES DESCRIPTION "Boost C++ Libraries" URL "http://www.boost.org" TYPE REQUIRED PURPOSE "Boost is required for building most KDEPIM applications")

# Kleopatra needs to know if the topological.hpp header exists (part of Boost_graph).
find_path(Boost_TOPOLOGICAL_SORT_DIR NAMES boost/graph/topological_sort.hpp PATHS ${Boost_INCLUDE_DIRS})
if(Boost_TOPOLOGICAL_SORT_DIR)
  message(STATUS "The Boost Topological_sort header was found. Building Kleopatra")
else()
  message(STATUS "The Boost Topological_sort header was NOT found. Kleopatra will not be built")
endif()

############### Windows specific ###############

if(WIN32)
  # detect oxygen icon dir at configure time based on KDEDIRS - there may be different package installation locations
  execute_process(COMMAND "${KDE4_KDECONFIG_EXECUTABLE}" --path icon OUTPUT_VARIABLE _dir ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
  file(TO_CMAKE_PATH "${_dir}" __dir)
  find_path(KDE4_ICON_DIR oxygen PATHS
    ${__dir}
  )
  message(STATUS "using oxygen application icons from ${KDE4_ICON_DIR}")
else()
    set (KDE4_ICON_DIR  ${CMAKE_INSTALL_PREFIX}/share/icons)
endif()

############### ONLY_KLEO ###############

# If the KDEPIM_ONLY_KLEO option is true
if(KDEPIM_ONLY_KLEO)
  find_package(QGpgme)
  set_package_properties(QGpgme PROPERTIES DESCRIPTION "The QGpgME library" URL "http://www.kde.org" TYPE REQUIRED PURPOSE "QGpgME is required to build Kleopatra.")

  add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${KDEPIM_DEFINITIONS})
  include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${KDEPIMLIBS_INCLUDE_DIRS} ${QT_QTDBUS_INCLUDE_DIR})
  include(kleopatra/ConfigureChecks.cmake)

  add_subdirectory(libkleo)
  if(Boost_TOPOLOGICAL_SORT_DIR)
    add_subdirectory(kleopatra)
  endif()
else()

# Otherwise...
############### Find the stuff we need ###############
  # Akonadi
  find_package(Akonadi 1.12.0 QUIET CONFIG)
  set_package_properties(Akonadi PROPERTIES DESCRIPTION "Akonadi server libraries" URL "http://pim.kde.org/akonadi" TYPE REQUIRED PURPOSE "Akonadi is required to build KDEPIM")

  find_package(ZLIB)
  set_package_properties(ZLIB PROPERTIES DESCRIPTION "The Zlib compression library" URL "http://www.zlib.net" TYPE REQUIRED)

  find_package(QGpgme)
  set_package_properties(QGpgme PROPERTIES DESCRIPTION "The QGpgMe library" URL "http://www.kde.org" TYPE RECOMMENDED PURPOSE "QGpgME is required to build KMail, KOrganizer and Kleopatra")

  find_package(Grantlee 0.3.0 QUIET CONFIG)
  set_package_properties(Grantlee PROPERTIES DESCRIPTION "The Grantlee Template System" URL "http://www.gitorious.org/grantlee/pages/Home" TYPE REQUIRED PURPOSE "Grantlee is requires for kmail and templating, theming for KJots, KaddressBook, KNotes and MessageViewer(KMail)." )

  find_package(Baloo 4.13.43 QUIET CONFIG)
  set_package_properties(Baloo PROPERTIES DESCRIPTION "The Baloo libraries" URL "http://www.kde.org" TYPE REQUIRED PURPOSE "Baloo provides search capabilities in KMail and Akonadi")

  # Xsltproc
  find_package(Xsltproc)
  set_package_properties(Xsltproc PROPERTIES DESCRIPTION "XSLT processor from libxslt" TYPE REQUIRED PURPOSE "Required to generate D-Bus interfaces.")

  find_package(QJSON)
  set_package_properties(QJSON PROPERTIES DESCRIPTION "QJSON" URL "http://qjson.sourceforge.net/" TYPE REQUIRED PURPOSE "Qt library for handling JSON data")

  find_package(Prison QUIET CONFIG)
  set_package_properties(Prison PROPERTIES DESCRIPTION "The Prison library" URL "http://projects.kde.org/prison" TYPE OPTIONAL PURPOSE "Needed to show mobile barcodes of your contacts")

  # Libkgapi2
  find_package(LibKGAPI2 2.1.0 QUIET CONFIG)
  set_package_properties(LibKGAPI2 PROPERTIES DESCRIPTION "KDE-based library for accessing various Google services" URL "https://projects.kde.org/libkgapi" TYPE OPTIONAL PURPOSE "LibKGAPI is required to build Google Drive Storage Service")

  if( LibKGAPI2_FOUND )
    add_definitions( -DKDEPIM_STORAGESERVICE_GDRIVE )
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

############### Needed commands before building anything ###############

  add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${KDEPIM_DEFINITIONS} ${AKONADI_DEFINITIONS})

  include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDEPIMLIBS_INCLUDE_DIRS} ${KDE4_INCLUDES} ${QT_QTDBUS_INCLUDE_DIR} ${Boost_INCLUDE_DIR})

  if(NOT KDEPIMLIBS_KRESOURCES_LIBRARY)
    add_definitions( -DKDEPIM_NO_KRESOURCES )
  endif()


############### Now, we add the PIM components ###############

  include (kleopatra/ConfigureChecks.cmake)

  # These targets will always be built before anything else.

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

  if(QGPGME_FOUND)
    if(Boost_TOPOLOGICAL_SORT_DIR)
      macro_optional_add_subdirectory(kleopatra)
    endif()
  endif()

  # The following components depend on QGpgME.
  add_subdirectory(messageviewer)
  macro_optional_add_subdirectory(messagecomposer)
  add_subdirectory(pimcommon)
  add_subdirectory(mailcommon) # TODO: does this make sense?!?
  macro_optional_add_subdirectory(kmail)
  macro_optional_add_subdirectory(grantleeeditor)
  if(KDEPIM_BUILD_MOBILE)
    add_subdirectory(mobile)
  endif()

  if(KDEPIM_BUILD_EXAMPLES)
    add_subdirectory(examples)
  endif()

  # If kmail is compiled, KMAIL_SUPPORTED is true (used in several places)
  if(BUILD_kmail)
    set(KMAIL_SUPPORTED TRUE)
    add_definitions(-DKMAIL_SUPPORTED)
  endif()

  macro_optional_add_subdirectory(korganizer)
  macro_optional_add_subdirectory(korgac)
  macro_optional_add_subdirectory(sieveeditor)
  macro_optional_add_subdirectory(storageservicemanager)
  add_subdirectory(kaddressbookgrantlee)
  if(KDEPIM_BUILD_DESKTOP)
    macro_optional_add_subdirectory(agents)
    macro_optional_add_subdirectory(akregator)
    macro_optional_add_subdirectory(importwizard)
    macro_optional_add_subdirectory(kaddressbook)
    macro_optional_add_subdirectory(kmailcvt)
    macro_optional_add_subdirectory(mboximporter)
    macro_optional_add_subdirectory(knotes)
    macro_optional_add_subdirectory(ksendemail)
    macro_optional_add_subdirectory(ktnef)
    macro_optional_add_subdirectory(mailimporter)
    macro_optional_add_subdirectory(pimsettingexporter)
    macro_optional_add_subdirectory(kalarm)

    if(KDEPIMLIBS_KCAL_LIBRARY)
      macro_optional_add_subdirectory(blogilo)
    endif()

    macro_optional_add_subdirectory(kjots)

    if(KDEPIMLIBS_KRESOURCES_LIBRARY)
      if(QT_QT3SUPPORT_FOUND)
        macro_optional_add_subdirectory(knode)
      endif()

      if(Q_WS_X11)
        macro_optional_add_subdirectory(ktimetracker)
      endif()

    endif()
    macro_optional_add_subdirectory(kontact) # must be the last one.
  endif()

  macro_optional_add_subdirectory(akonadiconsole)
  macro_optional_add_subdirectory(console)

  # These targets depend on optional applications
  if(KDEPIMLIBS_KRESOURCES_LIBRARY)
    add_subdirectory(kresources) # Must be after KAddressbook
  endif()

  add_subdirectory(plugins) # Must be after KMail

endif()

# doc must be a subdir of kdepim or packagers will kill us
macro_optional_add_subdirectory(doc)

# All done, let's display what we found...
feature_summary(WHAT ALL
                     INCLUDE_QUIET_PACKAGES
                     FATAL_ON_MISSING_REQUIRED_PACKAGES
               )

endif()
