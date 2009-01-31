project(kdepim)

option(KDE4_KDEPIM_NEW_DISTRLISTS "Whether to use new distribution lists, which stores them like normal contacts; useful for Kolab")
if(KDE4_KDEPIM_NEW_DISTRLISTS)
  add_definitions (-DKDEPIM_NEW_DISTRLISTS)
endif(KDE4_KDEPIM_NEW_DISTRLISTS)

add_definitions ( -DTest1Area=5121 ) # jstaniek

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

# search packages used by KDE
find_package (KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(CheckIncludeFiles)
find_package(KdepimLibs REQUIRED)

#####
##### Backwards compatibility section:
##### Support building against earlier kdelibs or kdepimlibs
#####
if(NOT DEFINED KDE4_KHOLIDAYS_LIBS)
  set(KDE4_KHOLIDAYS_LIBS kholidays)
endif(NOT DEFINED KDE4_KHOLIDAYS_LIBS)
#####

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

macro_optional_find_package(Boost)
macro_log_feature(Boost_FOUND "boost" "Boost C++ Libraries" "http://www.boost.org" TRUE "1.33.1" "Required by several critical KDEPIM apps.")
if(NOT Boost_FOUND)
  macro_display_feature_log()
endif(NOT Boost_FOUND)

if ( ONLY_KLEO )
  find_package(QGpgme REQUIRED)

  add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)

  include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${QT_QTDBUS_INCLUDE_DIR} )

  include (kleopatra/ConfigureChecks.cmake)

  add_subdirectory(libkleo)

  find_path(TOPOLOGICAL_SORT_DIR NAMES boost/graph/topological_sort.hpp PATHS ${Boost_INCLUDE_DIRS})
  if(TOPOLOGICAL_SORT_DIR)
    macro_optional_add_subdirectory(kleopatra)
  else(TOPOLOGICAL_SORT_DIR)
    message(STATUS "topological_sort.hpp is necessary to compile kleopatra. Please install package which contains it.")
  endif(TOPOLOGICAL_SORT_DIR)

else ( ONLY_KLEO )
  # regular case, build everything

  find_package(Akonadi REQUIRED)

  if ( NOT EXISTS ${CMAKE_SOURCE_DIR}/libkdepim/kdepim_export.h )
     message ( FATAL_ERROR "libkdepim directory not found" ) # needed for Kode to compile correctly
  endif ( NOT EXISTS ${CMAKE_SOURCE_DIR}/libkdepim/kdepim_export.h )
  find_package(Kode REQUIRED)
  find_package(ZLIB REQUIRED)
  find_package(Strigi REQUIRED)
  find_package(QGpgme)

  if(X11_FOUND)
    #X11_Xscreensaver discovery is done by FindX11
    macro_log_feature(X11_Xscreensaver_FOUND "X11 screensaver extension" "X Window Screen Saver Development Library" "http://xorg.freedesktop.org" FALSE "" "Required for building the KTimeTracker Program")
  endif(X11_FOUND)

# uncomment the following block for 4.3 in order to enable kitchensync
#  if (NOT WIN32)
#     set(OPENSYNC_MIN_VERSION 0.38)
#     macro_optional_find_package(OpenSync)
#     macro_log_feature(OPENSYNC_FOUND "opensync" "OpenSync Development Libraries" "http://www.opensync.org" FALSE "0.38 or greater" "Needed to provide syncing functionality in KDE PIM applications. Necessary to compile kitchensync.")
#     macro_optional_find_package(GLIB2)
#     macro_log_feature(GLIB2_FOUND "glib2" "Low-level core library for data structure handling, portability wrappers, etc." "http://www.gtk.org" FALSE "2.x" "Needed to provide synching applications from KDE PIM applications. Necessary to compile kitchensync and other PIM applications.")
#  endif (NOT WIN32)

  macro_optional_find_package(GNOKII)
  macro_bool_to_01(GNOKII_FOUND HAVE_GNOKII_H)
  macro_log_feature(GNOKII_FOUND "gnokii" "Gnokii Development Libraries" "http://www.gnokii.org" FALSE "" "Needed to provide the ability to import and export KDE addressbook entries to/from mobile phones that are accessible via gnokii")

  macro_optional_find_package(Soprano)
  macro_log_feature(Soprano_FOUND "Soprano" "Semantic Desktop Storing" "" FALSE "" "Soprano is needed for Nepomuk")

  macro_optional_find_package(Nepomuk)
  macro_log_feature(Nepomuk_FOUND "Nepomuk" "Nepomuk" "http://www.kde.org" FALSE "" "For adding desktop-wide tagging support to kmail")

  add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

  include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDEPIMLIBS_INCLUDE_DIR} ${KDE4_INCLUDES} ${QT_QTDBUS_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/libkdepim/)

  include (kleopatra/ConfigureChecks.cmake)

  set(libknotesresources_SRCS
     ${CMAKE_SOURCE_DIR}/knotes/resourcemanager.cpp
     ${CMAKE_SOURCE_DIR}/knotes/resourcenotes.cpp
     ${CMAKE_SOURCE_DIR}/knotes/resourcelocal.cpp
     ${CMAKE_SOURCE_DIR}/knotes/resourcelocalconfig.cpp
  )

  set(libknoteseditor_SRCS
    ${CMAKE_SOURCE_DIR}/knotes/knoteedit.cpp
  )

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

  add_subdirectory(kode)
  macro_optional_add_subdirectory(akonadi)
  add_subdirectory(libkdepim)
  add_subdirectory(kontactinterfaces)

  find_path(TOPOLOGICAL_SORT_DIR NAMES boost/graph/topological_sort.hpp PATHS ${Boost_INCLUDE_DIRS})
  if(QGPGME_FOUND)
     if(TOPOLOGICAL_SORT_DIR)
        macro_optional_add_subdirectory(kleopatra)
     else(TOPOLOGICAL_SORT_DIR)
        message(STATUS "topological_sort.hpp is necessary to compile kleopatra. Please install package which contains it.")
     endif(TOPOLOGICAL_SORT_DIR)
     add_subdirectory(libkleo)
  endif(QGPGME_FOUND)

  add_subdirectory(libkpgp)
  add_subdirectory(libksieve)
  add_subdirectory(mimelib)
  add_subdirectory(icons)
  macro_optional_add_subdirectory(akregator)
  macro_optional_add_subdirectory(knode)
  macro_optional_add_subdirectory(kjots)

  if(QGPGME_FOUND)
    set(KMAIL_SUPPORTED TRUE)
    add_definitions(-DKMAIL_SUPPORTED)
    message(STATUS "KMail support enabled.")
    macro_optional_add_subdirectory(kmail)
  else(QGPGME_FOUND)
    set(KMAIL_SUPPORTED FALSE)
    message(STATUS "KMail support DISABLED (QGPGME not found).")
  endif(QGPGME_FOUND)

  macro_optional_add_subdirectory(kalarm)
  macro_optional_add_subdirectory(kaddressbook)
#  macro_optional_add_subdirectory(kcontactmanager)

  if(Q_WS_X11)
    set(KTIMETRACKER_SUPPORTED TRUE)
    add_definitions(-DKTIMETRACKER_SUPPORTED)
    message(STATUS "KTimeTracker support enabled.")
    macro_optional_add_subdirectory(ktimetracker)
  else(Q_WS_X11)
    set(KTIMETRACKER_SUPPORTED FALSE)
    message(STATUS "KTimeTracker support DISABLED (non-X11 target).")
  endif (Q_WS_X11)

  add_subdirectory(kdgantt1)

  macro_optional_add_subdirectory(strigi-analyzer)

  if(QGPGME_FOUND AND KMAIL_SUPPORTED)
     macro_optional_add_subdirectory(kmailcvt)
     message(STATUS "KMailCVT support enabled.")
  else(QGPGME_FOUND AND KMAIL_SUPPORTED)
     message(STATUS "KMailCVT support DISABLED (because KMail support is disabled).")
  endif(QGPGME_FOUND AND KMAIL_SUPPORTED)
  macro_optional_add_subdirectory(knotes)
  macro_optional_add_subdirectory(console)

  if(QGPGME_FOUND AND KMAIL_SUPPORTED)
     macro_optional_add_subdirectory(korganizer)
     message(STATUS "KOrganizer support enabled.")
  else(QGPGME_FOUND AND KMAIL_SUPPORTED)
     message(STATUS "KOrganizer support DISABLED (because KMail support is disabled).")
  endif(QGPGME_FOUND AND KMAIL_SUPPORTED)

  if(KMAIL_SUPPORTED)
     macro_optional_add_subdirectory(ksendemail)
     message(STATUS "KSendEmail support enabled.")
  else(KMAIL_SUPPORTED)
     message(STATUS "KSendEmail support DISABLED (because KMail support is disabled).")
  endif(KMAIL_SUPPORTED)

  if(NOT WIN32)
#enable kmobiletools 4.3 if there is interest
#     set(KMOBILETOOLS_SUPPORTED TRUE)
#     add_definitions(-DKMOBILETOOLS_SUPPORTED)
#     message(STATUS "KMobileTools support enabled.")
#     macro_optional_add_subdirectory(kmobiletools)
     set(KMOBILETOOLS_SUPPORTED FALSE)

     set(KPILOT_SUPPORTED TRUE)
     add_definitions(-DKPILOT_SUPPORTED)
     message(STATUS "KPilot support enabled.")
     macro_optional_add_subdirectory(kpilot)

#     if(OPENSYNC_FOUND AND GLIB2_FOUND)
#       set(KITCHENSYNC_SUPPORTED TRUE)
#       add_definitions(-DKITCHENSYNC_SUPPORTED)
#       message(STATUS "KitchenSync support enabled.")
#       macro_optional_add_subdirectory(kitchensync)
#     else(OPENSYNC_FOUND AND GLIB2_FOUND)
#       set(KITCHENSYNC_SUPPORTED FALSE)
#       if(NOT OPENSYNC_FOUND)
#         message(STATUS "KitchenSync support disabled (OpenSync not found).")
#       endif(NOT OPENSYNC_FOUND)
#       if(NOT GLIB2_FOUND)
#         message(STATUS "KitchenSync support disabled (GLIB not found).")
#       endif(NOT GLIB2_FOUND)
#     endif(OPENSYNC_FOUND AND GLIB2_FOUND)
     set(KITCHENSYNC_SUPPORTED FALSE)
  else(NOT WIN32)
     set(KMOBILETOOLS_SUPPORTED FALSE)
     message(STATUS "KMobileTools support DISABLED (Windows target).")
     set(KPILOT_SUPPORTED FALSE)
     message(STATUS "KPilot support DISABLED (Windows target).")
     set(KITCHENSYNC_SUPPORTED FALSE)
     message(STATUS "KitchenSync support DISABLED (Windows target).")
  endif(NOT WIN32)

  add_subdirectory(akonadiconsole)
#enable korn in 4.3 if there is interest
# macro_optional_add_subdirectory(korn)
  add_subdirectory(kresources)
  macro_optional_add_subdirectory(kontact)

  if(QGPGME_FOUND)
     add_subdirectory(wizards)
  endif(QGPGME_FOUND)

  add_subdirectory(cmake)

  add_subdirectory(plugins)

endif( ONLY_KLEO )

macro_optional_add_subdirectory(doc)
macro_display_feature_log()
