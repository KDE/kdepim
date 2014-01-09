project(kdepim)

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

add_definitions( -DDISABLE_NEPOMUK_LEGACY )
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
if(KDEPIM_NO_WEBKIT)
  add_definitions(-DKDEPIM_NO_WEBKIT)
endif()

# cmake would not find kde for xcompile if that is not set
if(WINCE)
    set(KDE4_DATA_DIR "${CMAKE_PREFIX_PATH}/share/apps")
    set(LIBRARY_TYPE STATIC)
    add_definitions(-DKDELIBS_STATIC_LIBS)
    add_definitions(-DKDEPIM_STATIC_LIBS)
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
set(KDEPIM_DEV_VERSION)

# add an extra space
if(DEFINED KDEPIM_DEV_VERSION)
  set(KDEPIM_DEV_VERSION " ${KDEPIM_DEV_VERSION}")
endif()

set(KDEPIM_VERSION "4.12.1${KDEPIM_DEV_VERSION}")

configure_file(kdepim-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdepim-version.h @ONLY)

############### search packages used by KDE ###############

# It is needed otherwise it finds the kde4-config of the target platform
if(WINCE)
  find_program(KDE4_KDECONFIG_EXECUTABLE NAMES kde4-config PATHS ${HOST_BINDIR} NO_DEFAULT_PATH)
endif()

find_package(KDE4 4.11.3 REQUIRED)
include(KDE4Defaults)

find_package(KdepimLibs 4.12.1)
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
  find_package(Akonadi 1.10.45 QUIET CONFIG)
  set_package_properties(Akonadi PROPERTIES DESCRIPTION "Akonadi server libraries" URL "http://pim.kde.org/akonadi" TYPE REQUIRED PURPOSE "Akonadi is required to build KDEPIM")

  find_package(ZLIB)
  set_package_properties(ZLIB PROPERTIES DESCRIPTION "The Zlib compression library" URL "http://www.zlib.net" TYPE REQUIRED)

  # Strigi is required only if AKONADI_USE_STRIGI_SEARCH is true in AkonadiConfig.cmake
  if(AKONADI_USE_STRIGI_SEARCH)
    find_package(Strigi)
    set_package_properties(Strigi PROPERTIES URL "https://projects.kde.org/strigi" TYPE REQUIRED PURPOSE "Metadata indexing")
  endif()

  find_package(QGpgme)
  set_package_properties(QGpgme PROPERTIES DESCRIPTION "The QGpgMe library" URL "http://www.kde.org" TYPE RECOMMENDED PURPOSE "QGpgME is required to build KMail, KOrganizer and Kleopatra")

  find_package(SharedDesktopOntologies 0.11)
  set_package_properties(SharedDesktopOntologies PROPERTIES DESCRIPTION "Shared Desktop Ontologies" URL "http://sourceforge.net/projects/oscaf/" TYPE REQUIRED PURPOSE "Ontologies necessary for the Nepomuk semantic desktop")

  find_package(Soprano 2.9.0)
  set_package_properties(Soprano PROPERTIES DESCRIPTION "C++/Qt4 framework for RDF data" URL "http://soprano.sourceforge.net" TYPE REQUIRED PURPOSE "Storage of semantic data")
  include(SopranoAddOntology)

  find_package(Grantlee 0.3.0 QUIET CONFIG)
  set_package_properties(Grantlee PROPERTIES DESCRIPTION "The Grantlee Template System" URL "http://www.gitorious.org/grantlee/pages/Home" TYPE REQUIRED PURPOSE "Grantlee is requires for kmail and templating, theming for KJots, KaddressBook, KNotes and MessageViewer(KMail)." )

  find_package(NepomukCore QUIET CONFIG)
  set_package_properties(NepomukCore PROPERTIES DESCRIPTION "The Nepomuk Core libraries" URL "http://www.kde.org" TYPE REQUIRED PURPOSE "Nepomuk extends the search and tagging functionalities in KMail and Akonadi")

  find_package(NepomukWidgets QUIET CONFIG)
  set_package_properties(NepomukWidgets PROPERTIES DESCRIPTION "The NepomukWidget libraries" URL "http://www.kde.org" TYPE RECOMMENDED PURPOSE "Nepomuk-widgets is needed for building Akonadiconsole")

  # Xsltproc
  find_package(Xsltproc)
  set_package_properties(Xsltproc PROPERTIES DESCRIPTION "XSLT processor from libxslt" TYPE REQUIRED PURPOSE "Required to generate D-Bus interfaces.")

  find_package(QJSON)
  set_package_properties(QJSON PROPERTIES DESCRIPTION "QJSON" URL "http://qjson.sourceforge.net/" TYPE REQUIRED PURPOSE "Qt library for handling JSON data")

  find_package(Prison QUIET CONFIG)
  set_package_properties(Prison PROPERTIES DESCRIPTION "The Prison library" URL "http://projects.kde.org/prison" TYPE OPTIONAL PURPOSE "Needed to show mobile barcodes of your contacts")


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

  add_subdirectory(ontologies) # must be the first one.
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
  add_subdirectory(grammar)
  add_subdirectory(messagecore)
  add_subdirectory(grantleetheme)
  if(NOT WINCE)
    add_subdirectory(messagelist)
  else()
    # We need to compile runtime in kdepim because of the static
    # linking of korgac and the applications need to link against
    # plugins like the serializer plugins. If the korgac would
    # move to runtime this can be avoided
    add_subdirectory(runtime)
  endif()
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
  macro_optional_add_subdirectory(headerthemeeditor)
  macro_optional_add_subdirectory(contactthemeeditor)
  add_subdirectory(grantleethemeeditor)

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

  macro_optional_add_subdirectory(pimactivity)
  macro_optional_add_subdirectory(korganizer)
  macro_optional_add_subdirectory(korgac)
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

  if(NepomukWidgets_FOUND)
    macro_optional_add_subdirectory(akonadiconsole)
  endif()
  macro_optional_add_subdirectory(console)
  if(Strigi_FOUND)
    macro_optional_add_subdirectory(strigi-analyzer)
  endif()

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

