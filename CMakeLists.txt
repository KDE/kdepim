project(kdepim)

# where to look first for cmake modules. This line must be the first one or cmake will use the system's FindFoo.cmake
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

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
  find_program(GIT_EXECUTABLE NAMES git)
  if(GIT_EXECUTABLE)
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
# If "z" is 0, it the version is "x.y"
#
# KDEPIM_DEV_VERSION
# is empty for final versions. For development versions "build" is
# something like "pre", "alpha1", "alpha2", "beta1", "beta2", "rc1", "rc2".
#
# Examples in chronological order:
#
#    3.0
#    3.0.1
#    3.1 alpha1
#    3.1 beta1
#    3.1 beta2
#    3.1 rc1
#    3.1
#    3.1.1
#    3.2 pre
#    3.2 alpha1

# Do NOT add quote
set(KDEPIM_DEV_VERSION)

# add an extra space
if(DEFINED KDEPIM_DEV_VERSION)
  set(KDEPIM_DEV_VERSION " ${KDEPIM_DEV_VERSION}")
endif()

set(KDEPIM_VERSION "4.10.5${KDEPIM_DEV_VERSION}")

configure_file(kdepim-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdepim-version.h @ONLY)

############### search packages used by KDE ###############

# It is needed otherwise it finds the kde4-config of the target platform
if(WINCE)
  find_program(KDE4_KDECONFIG_EXECUTABLE NAMES kde4-config PATHS ${HOST_BINDIR} NO_DEFAULT_PATH)
endif()

find_package(KDE4 4.7.0 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(CheckIncludeFiles)
include(MacroBoolTo01)
find_package(KdepimLibs 4.10.3 REQUIRED)

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
macro_log_feature(Boost_FOUND "Boost" "Boost C++ Libraries" "http://www.boost.org" TRUE "1.34.0" "Required by several critical KDEPIM apps.")

# Kleopatra needs to know if the topological.hpp header exists (part of Boost_graph).
find_path(Boost_TOPOLOGICAL_SORT_DIR NAMES boost/graph/topological_sort.hpp PATHS ${Boost_INCLUDE_DIRS})
macro_log_feature(Boost_TOPOLOGICAL_SORT_DIR "The Boost Topological_sort header" "topological_sort.hpp (part of Boost_graph)" "http://www.boost.org/" FALSE "" "Necessary to build kleopatra")

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

# The KDEPIM_ONLY_KLEO option is true
if(KDEPIM_ONLY_KLEO)
  find_package(QGpgme)
  macro_log_feature(QGPGME_FOUND "QGpgME" "The QGpgME library" "http://www.kde.org" TRUE "" "QGpgME is required to build Kleopatra.")

  add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${KDEPIM_DEFINITIONS} -DQT3_SUPPORT_WARNINGS)
  include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${KDEPIMLIBS_INCLUDE_DIRS} ${QT_QTDBUS_INCLUDE_DIR} ${NEPOMUK_CORE_INCLUDE_DIR})
  include(kleopatra/ConfigureChecks.cmake)

  add_subdirectory(libkleo)
  if(Boost_TOPOLOGICAL_SORT_DIR)
    add_subdirectory(kleopatra)
  endif()
else()

# Otherwise...
############### Find the stuff we need ###############
  set(AKONADI_MIN_VERSION 1.8.51)
  find_package(Akonadi QUIET NO_MODULE ${AKONADI_MIN_VERSION})
  macro_log_feature(Akonadi_FOUND "Akonadi" "Akonadi server libraries" "http://pim.kde.org/akonadi" TRUE "${AKONADI_MIN_VERSION}" "Akonadi is required to build KDEPIM")

  find_package(ZLIB)
  macro_log_feature(ZLIB_FOUND "ZLib" "The Zlib compression library" "http://www.zlib.net" TRUE "" "")

  find_package(Strigi)
  macro_log_feature(STRIGI_FOUND "Strigi" "Index metadata of files" "http://strigi.sourceforge.net" TRUE "" "")

  find_package(QGpgme)
  macro_log_feature(QGPGME_FOUND "QGpgME" "The QGpgME library" "http://www.kde.org" FALSE "" "QGpgME is required to build KMail, KOrganizer and Kleopatra")

  set(SDO_MIN_VERSION 0.10)
  set(SOPRANO_MIN_VERSION 2.7.56)

  find_package(SharedDesktopOntologies ${SDO_MIN_VERSION})
  macro_log_feature(SHAREDDESKTOPONTOLOGIES_FOUND "Shared desktop ontologies" "Desktop ontologies" "http://oscaf.sourceforge.net" TRUE "${SDO_MIN_VERSION}" "Ontologies necessary for the Nepomuk semantic desktop.")

  find_package(Soprano)
  macro_log_feature(Soprano_FOUND "Soprano" "Semantic Desktop Storing" "http://soprano.sourceforge.net" TRUE "${SOPRANO_MIN_VERSION}" "Soprano is needed for Nepomuk")
  include(SopranoAddOntology)

  find_package(Grantlee QUIET NO_MODULE)
  macro_log_feature(Grantlee_FOUND "Grantlee" "The Grantlee Template System" "http://www.gitorious.org/grantlee/pages/Home" FALSE "" "Templating and theming for KJots and KaddressBook." )

  if(${Grantlee_FOUND} AND ("${Grantlee_VERSION}" VERSION_GREATER "0.2.0"))
     set(GRANTLEE_GREATER_0_2 TRUE)
  endif()

  find_package(NepomukCore)
  macro_log_feature(NepomukCore_FOUND "NepomukCore" "The NepomukCore libraries" "http://www.kde.org" TRUE "" "Nepomuk extends the search and tagging functionalities in KMail and Akonadi")

  find_package(NepomukWidgets)
  macro_log_feature(NepomukWidgets_FOUND "NepomukWidgets" "The NepomukWidget libraries" "http://www.kde.org" TRUE "" "Nepomuk extends the search and tagging functionalities in AkonadiConsole")


  find_package(LibXslt)
  macro_log_feature(XSLTPROC_EXECUTABLE "xsltproc" "The command line XSLT processor from libxslt" "http://xmlsoft.org/XSLT/" TRUE "" "Required to generate the D-Bus interfaces.")

############### Desktop vs. Mobile options ##############

  if(KDEPIM_MOBILE_UI)
    macro_log_feature(QT_QTDECLARATIVE_FOUND "QtDeclarative" "Qt Declarative (QML) module" "http://qt.nokia.com" TRUE "4.7" "Required for building mobile UI")
    add_definitions( -DKDEPIM_MOBILE_UI )
  else()
    macro_log_feature(QT_QTDECLARATIVE_FOUND "QtDeclarative" "Qt Declarative (QML) module" "http://qt.nokia.com" FALSE "4.7" "Required for building the mobile applications")
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

############### Macros ###############

  # generates a D-Bus interface description from a KConfigXT file
  macro(kcfg_generate_dbus_interface _kcfg _name)
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_name}.xml
      COMMAND ${XSLTPROC_EXECUTABLE} --stringparam interfaceName ${_name}
      ${KDEPIMLIBS_DATA_DIR}/akonadi-kde/kcfg2dbus.xsl
      ${_kcfg}
      > ${CMAKE_CURRENT_BINARY_DIR}/${_name}.xml
      DEPENDS ${KDEPIMLIBS_DATA_DIR}/akonadi-kde/kcfg2dbus.xsl
      ${_kcfg}
      )
  endmacro(kcfg_generate_dbus_interface)

############### Code used in several places ###############
# TODO Remove from the top CMakeLists.

# used in knotes, kontact/plugins/knotes, wizard
  set(libknotesresources_SRCS
    ${CMAKE_SOURCE_DIR}/knotes/resourcemanager.cpp
    ${CMAKE_SOURCE_DIR}/knotes/resourcenotes.cpp
    ${CMAKE_SOURCE_DIR}/knotes/resourcelocal.cpp
    ${CMAKE_SOURCE_DIR}/knotes/resourcelocalconfig.cpp
  )

# used in knotes, kontact/plugins/knotes
  set(libknoteseditor_SRCS
    ${CMAKE_SOURCE_DIR}/knotes/knoteedit.cpp
  )

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
  #add_subdirectory(composereditor-ng)
  add_subdirectory(messagecore)
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
  set(MESSAGEVIEWER_SUPPORTED TRUE)
  add_subdirectory(messageviewer)
  macro_optional_add_subdirectory(messagecomposer)
  add_subdirectory(pimcommon)
  add_subdirectory(mailcommon) # TODO: does this make sense?!?
  add_subdirectory(mailfilteragent)
  macro_optional_add_subdirectory(kmail)

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
  if(KDEPIM_BUILD_DESKTOP)
    macro_optional_add_subdirectory(akregator)
    macro_optional_add_subdirectory(archivemailagent)
    macro_optional_add_subdirectory(importwizard)
    macro_optional_add_subdirectory(kaddressbook)
    macro_optional_add_subdirectory(kmailcvt)
    macro_optional_add_subdirectory(knotes)
    macro_optional_add_subdirectory(ksendemail)
    macro_optional_add_subdirectory(ktnef)
    macro_optional_add_subdirectory(mailimporter)
    macro_optional_add_subdirectory(pimsettingexporter)
    macro_optional_add_subdirectory(kalarm)

    if(KDEPIMLIBS_KCAL_LIBRARY)
      macro_optional_add_subdirectory(blogilo)
    endif()

    if(Grantlee_FOUND)
      macro_optional_add_subdirectory(kjots)
    endif()

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
  macro_optional_add_subdirectory(strigi-analyzer)

  # These targets depend on optional applications
  if(KDEPIMLIBS_KRESOURCES_LIBRARY)
    add_subdirectory(kresources) # Must be after KAddressbook
  endif()

  add_subdirectory(plugins) # Must be after KMail

endif()

# doc must be a subdir of kdepim or packagers will kill us
macro_optional_add_subdirectory(doc)


# We really want to encourage users to enable/install QGpgME from kdepimlibs
if(NOT QGPGME_FOUND)
  # Users must be aware that QGpgMe is really needed
  message(STATUS "*** WARNING: QGpgME is not installed on your system ***")
  message(STATUS "*** It is required if you want to use KMail, KOrganizer or Kleopatra ***")
  message(STATUS "*** You are really encouraged to install it ***")
endif()

# All done, let's display what we found...
macro_display_feature_log()
