project(kdepim)

# where to look first for cmake modules. This line must be the first one or cmake will use the system's FindFoo.cmake
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")


############### Build Options ###############

option(KDEPIM_ENTERPRISE_BUILD "Enable features specific to the enterprise branch, which are normally disabled. Also, it disables many components not needed for Kontact such as the Kolab client." FALSE)
option(KDEPIM_ONLY_KLEO "Only build Kleopatra. This option will disable KDEPIM_BUILD_EVERYTHING and KDEPIM_ENTERPRISE_BUILD." FALSE)
option(KDEPIM_BUILD_KCONTACTMANAGER "Build KcontactManager which will replace KAddressBook in KDE 4.4." FALSE)
# if KDEPIM_ENTERPRISE_BUILD is defined, KDEPIM_ONLY_KLEO is disabled
if(KDEPIM_ENTERPRISE_BUILD)
  set(KDEPIM_ONLY_KLEO FALSE)
  set(KDEPIM_BUILD_EVERYTHING FALSE)
  message(STATUS "Enterprise build is enabled.")
else(KDEPIM_ENTERPRISE_BUILD)
  # if KDEPIM_ONLY_KLEO is defined, KDEPIM_BUILD_EVERYTHING is disabled.
  if(KDEPIM_ONLY_KLEO)
    set(KDEPIM_BUILD_EVERYTHING FALSE)
    set(KDEPIM_DEFINITIONS "-DHAVE_CONFIG_H=1")
    message(STATUS "Only libkleo and Kleopatra will be built.")
  endif(KDEPIM_ONLY_KLEO)
  # Default
    set(KDEPIM_BUILD_EVERYTHING TRUE)
    message(STATUS "Enterprise build is disabled.")
endif(KDEPIM_ENTERPRISE_BUILD)

# config-enterprise.h is needed for both ENTERPRISE_BUILD and BUILD_EVERYTHING
configure_file(config-enterprise.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-enterprise.h )


############### search packages used by KDE ###############

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(CheckIncludeFiles)
find_package(KdepimLibs 4.2.85 REQUIRED)


############### search Boost ###############

set(Boost_MINIMUM_VERSION "1.33.1")
find_package(Boost)
macro_log_feature(Boost_FOUND "boost" "Boost C++ Libraries" "http://www.boost.org" TRUE "" "Required by several critical KDEPIM apps.")

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
else(WIN32)
    set (KDE4_ICON_DIR  ${CMAKE_INSTALL_PREFIX}/share/icons)
endif(WIN32)


############### ONLY_KLEO ###############

# The KDEPIM_ONLY_KLEO option is true
if(KDEPIM_ONLY_KLEO)
  find_package(QGpgme)
  macro_log_feature(QGPGME_FOUND "qgpgme" "The QGpgME library." "http://www.kde.org" TRUE "" "QGpgME is required to build Kleopatra.")

  add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${KDEPIM_DEFINITIONS})
  include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${KDEPIMLIBS_INCLUDE_DIRS} ${QT_QTDBUS_INCLUDE_DIR})
  include(kleopatra/ConfigureChecks.cmake)

  add_subdirectory(libkleo)
  if (Boost_TOPOLOGICAL_SORT_DIR)
    macro_optional_add_subdirectory(kleopatra)
  endif (Boost_TOPOLOGICAL_SORT_DIR)

else(KDEPIM_ONLY_KLEO)

# Otherwise...
############### Find the stuff we need ###############

  find_package(Akonadi)
  macro_log_feature(Akonadi_FOUND "Akonadi" "Akonadi client libraries (from kdepimlibs)" "http://pim.kde.org/akonadi" TRUE "" "Akonadi is required to build KDEPIM")

  find_package(ZLIB)
  macro_log_feature(ZLIB_FOUND "zlib" "The Zlib compression library" "http://www.zlib.net" TRUE "" "")

  find_package(Strigi)
  macro_log_feature(STRIGI_FOUND "Strigi" "Index metadata of files" "http:// strigi.sourceforge.net" TRUE "" "")

  find_package(QGpgme)
  macro_log_feature(QGPGME_FOUND "QGpgME" "The QGpgME library." "http://www.kde.org" FALSE "" "QGpgME is required to build KMail, KOrganizer and Kleopatra")

  macro_optional_find_package(Soprano)
  macro_log_feature(Soprano_FOUND "Soprano" "Semantic Desktop Storing" FALSE "" "" "Soprano is needed for Nepomuk")

  macro_optional_find_package(Nepomuk)
  macro_log_feature(Nepomuk_FOUND "Nepomuk" "The Nepomuk libraries" "http://www.kde.org" FALSE "" "Nepomuk extends the search and tagging functionalities in KMail and Akonadi")

  find_package(LibKNotificationItem-1)
  macro_log_feature(LIBKNOTIFICATIONITEM-1_FOUND "LibKNotificationItem-1" "An experimental library" "svn://anonsvn.kde.org/home/kde/trunk/KDE/kdelibs/experimental" TRUE "" "LibKNotificationItem-1 is an unfinished library which is required by korgac and akonaditray.")


############### Needed commands before building anything ###############

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${KDEPIM_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDEPIMLIBS_INCLUDE_DIRS} ${KDE4_INCLUDES} ${QT_QTDBUS_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/libkdepim)


############### Code used in several places ###############
# TODO Remove from the top CMakeLists.

# used in knotes, kontact/plugins/knotes, kresources/egroupware, kresources/kolab/knotes, kresources/scalix/knotes, wizard
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

# used in kdgantt1, korganizer
set(libkdgantt1_SRCS
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttView.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewEventItem.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewItem.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewItemDrag.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewSubwidgets.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewSummaryItem.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewTaskItem.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewTaskLink.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewTaskLinkGroup.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttMinimizeSplitter.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttSemiSizingControl.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttSizingControl.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttXMLTools.cpp
)


############### Now, we add the PIM components ###############

  include (kleopatra/ConfigureChecks.cmake)

  # These targets will always be built
  add_subdirectory(libkdepim)
  add_subdirectory(libkleo)
  add_subdirectory(libkpgp)
  add_subdirectory(libksieve)
  add_subdirectory(kontactinterfaces)
  add_subdirectory(mimelib)
  add_subdirectory(kresources)
  add_subdirectory(kdgantt1)
  add_subdirectory(akonadiconsole)
  add_subdirectory(icons)
  add_subdirectory(plugins)

  if(QGPGME_FOUND)
    if (Boost_TOPOLOGICAL_SORT_DIR)
      macro_optional_add_subdirectory(kleopatra)
    endif(Boost_TOPOLOGICAL_SORT_DIR)
    add_subdirectory(wizards)
    # The following components depends on QGpgME.
    macro_optional_add_subdirectory(kmail)
    # If kmail is compiled, KMAIL_SUPPORTED is true (used in several places)
    macro_bool_to_01(BUILD_kmail KMAIL_SUPPORTED)
    macro_optional_add_subdirectory(kmailcvt)
    macro_optional_add_subdirectory(ksendemail)
    macro_optional_add_subdirectory(korganizer)
  endif(QGPGME_FOUND)

  if(KDEPIM_BUILD_EVERYTHING)
    macro_optional_add_subdirectory(akonadi)
    macro_optional_add_subdirectory(knode)
  endif(KDEPIM_BUILD_EVERYTHING)

  macro_optional_add_subdirectory(akregator)
  macro_optional_add_subdirectory(console)
  macro_optional_add_subdirectory(kaddressbook)
  macro_optional_add_subdirectory(kalarm)
  if(KDEPIM_BUILD_KCONTACTMANAGER)
    macro_optional_add_subdirectory(kcontactmanager)
  endif(KDEPIM_BUILD_KCONTACTMANAGER)
  macro_optional_add_subdirectory(kjots)
  macro_optional_add_subdirectory(knotes)
  macro_optional_add_subdirectory(kontact)
  macro_optional_add_subdirectory(strigi-analyzer)

  if(Q_WS_X11)
    macro_optional_add_subdirectory(ktimetracker)
  endif(Q_WS_X11)

  # Kpilot won't build on Windows
  if(NOT WIN32)
    macro_optional_add_subdirectory(kpilot)

    #if(OPENSYNC_FOUND AND GLIB2_FOUND)
    #  macro_optional_add_subdirectory(kitchensync)
    #endif(OPENSYNC_FOUND AND GLIB2_FOUND)
  endif(NOT WIN32)

endif (KDEPIM_ONLY_KLEO)


# doc must be a subdir of kdepim or packagers will kill us
macro_optional_add_subdirectory(doc)


# We really want to encourage users to enable/install QGpgME from kdepimlibs
if(NOT QGPGME_FOUND)
  # Users must be aware that QGpgMe is really needed
  message(STATUS "*** WARNING: QGpgME is not installed on your system ***")
  message(STATUS "*** It is required if you want to use KMail, KOrganizer or Kleopatra ***")
  message(STATUS "*** You are really encouraged to install it ***")
endif(NOT QGPGME_FOUND)
# All done, let's display what we found...
macro_display_feature_log()
