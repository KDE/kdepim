project(kdepim)

option(ENTERPRISE_BUILD "Enable features specific to the enterprise branch, which are normally disabled. Also, it disabled many components not needed for Kontact as the Kolab client." TRUE)
if(ENTERPRISE_BUILD)
  set(BUILD_EVERYTHING FALSE)
  message(STATUS "Enterprise build is enabled.")
else(ENTERPRISE_BUILD)
  set(BUILD_EVERYTHING TRUE)
  message(STATUS "Enterprise build is disabled.")
endif(ENTERPRISE_BUILD)
configure_file(config-enterprise.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-enterprise.h )

option(KDE4_KDEPIM_NEW_DISTRLISTS "Whether to use new distribution lists, which stores them like normal contacts; useful for Kolab")
if(KDE4_KDEPIM_NEW_DISTRLISTS)
  add_definitions (-DKDEPIM_NEW_DISTRLISTS)
endif(KDE4_KDEPIM_NEW_DISTRLISTS)

add_definitions ( -DTest1Area=5121 ) # jstaniek

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

# search packages used by KDE
find_package (KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(MacroOptionalAddSubdirectory)
include(CheckIncludeFiles)
include(FindKdepimLibs)

if(WIN32)
    set(LIB_INSTALL_DIR ${LIB_INSTALL_DIR}
                        RUNTIME DESTINATION ${BIN_INSTALL_DIR}
                        LIBRARY DESTINATION ${LIB_INSTALL_DIR}
                        ARCHIVE DESTINATION ${LIB_INSTALL_DIR} )
endif(WIN32)

set(KDE4_ICON_DIR ${KDE4_INSTALL_DIR}/share/icons)

# this macro exists to work around a stupid Mac OS X linker bug
# where it can't handle the same library being referenced multiple
# times on the linker line
macro (KDEPIM4_LINK_UNIQUE_LIBRARIES)
  set(LINK_ARG_LIST)  
  foreach(arg ${ARGN})
    set(LINK_ARG_FOUND FALSE)
    foreach(existing ${LINK_ARG_LIST})
      if(${arg} STREQUAL ${existing})
        set(LINK_ARG_FOUND)
      endif(${arg} STREQUAL ${existing})
    endforeach(existing)

    if(NOT ${LINK_ARG_FOUND})
      list(APPEND LINK_ARG_LIST ${arg})
    endif(NOT ${LINK_ARG_FOUND})
  endforeach(arg)

  target_link_libraries(${LINK_ARG_LIST})
endmacro (KDEPIM4_LINK_UNIQUE_LIBRARIES)

find_package(Boost REQUIRED)
macro_log_feature(Boost_FOUND "Boost" "Boost C++ Libraries" "http://www.boost.org/" TRUE "" "Required by kleopatra")

if ( ONLY_KLEO )
  find_package(QGpgme REQUIRED)

  add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)

  include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${QT_QTDBUS_INCLUDE_DIR} )

  include (kleopatra/ConfigureChecks.cmake)

  add_subdirectory(libkleo)

  find_path(TOPOLOGICAL_SORT_DIR NAMES boost/graph/topological_sort.hpp PATHS ${Boost_INCLUDE_DIRS})
     macro_log_feature(TOPOLOGICAL_SORT_DIR "Boost Topological Sort" "Missing Boost header topological.h" "http://www.boost.org/" FALSE "" "Necessary to build kleopatra")
  if(TOPOLOGICAL_SORT_DIR)
    macro_optional_add_subdirectory(kleopatra)
  endif(TOPOLOGICAL_SORT_DIR)
  

  macro_optional_add_subdirectory(doc)

else ( ONLY_KLEO )
  # regular case, build everything
  find_package(Kode REQUIRED)
  find_package(ZLIB REQUIRED)
  find_package(Strigi REQUIRED)
  find_package(QGpgme)

  if(Q_WS_X11)
    macro_log_feature(X11_Xscreensaver_LIB "libXss" "X Window Screen Saver Development Library" "http://xorg.freedesktop.org" FALSE "" "Required for building the KTimeTracker Program")
  endif(Q_WS_X11)

  macro_optional_find_package(GNOKII)
  macro_bool_to_01(GNOKII_FOUND HAVE_GNOKII_H)
  macro_log_feature(GNOKII_FOUND "gnokii" "Gnokii Development Libraries" "http://www.gnokii.org" FALSE "" "Needed to provide the ability to import and export KDE addressbook entries to/from mobile phones that are accessible via gnokii")

  macro_optional_find_package(Soprano)
  macro_log_feature(Soprano_FOUND "Soprano" "Semantic Desktop Storing" "" FALSE "" "Soprano is needed for Nepomuk")

  macro_optional_find_package(Nepomuk)
  macro_log_feature(Nepomuk_FOUND "Nepomuk" "Nepomuk" "http://www.kde.org" FALSE "" "For adding desktop-wide tagging support to kmail")

  find_program(XSLTPROC_EXECUTABLE xsltproc)

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

  macro_optional_add_subdirectory(doc)
  add_subdirectory(libkdepim)
  add_subdirectory(libkholidays)
  add_subdirectory(kontactinterfaces)

  if(QGPGME_FOUND)
     find_path(TOPOLOGICAL_SORT_DIR NAMES boost/graph/topological_sort.hpp PATHS ${Boost_INCLUDE_DIRS})
     macro_log_feature(TOPOLOGICAL_SORT_DIR "Boost Topological Sort" "Missing Boost header topological.h" "http://www.boost.org/" FALSE "" "Necessary to build kleopatra")
     if(TOPOLOGICAL_SORT_DIR)
        macro_optional_add_subdirectory(kleopatra)
     endif(TOPOLOGICAL_SORT_DIR)
     add_subdirectory(libkleo)
  endif(QGPGME_FOUND)

  add_subdirectory(libkpgp)
  add_subdirectory(libksieve)
  add_subdirectory(mimelib)
  add_subdirectory(icons)
  macro_optional_add_subdirectory(akregator)
  if(BUILD_EVERYTHING)
    macro_optional_add_subdirectory(knode)
  endif(BUILD_EVERYTHING)
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
     macro_optional_find_package(Opensync)
     macro_log_feature(OPENSYNC_FOUND "opensync" "OpenSync Development Libraries" "http://www.opensync.org" FALSE "0.30 or greater" "Needed to provide syncing functionality in KDE PIM applications. Necessary to compile kitchensync.")
     macro_optional_find_package(GLIB2)
     macro_log_feature(GLIB2_FOUND "glib2" "Low-level core library for data structure handling, portability wrappers, etc." "http://www.gtk.org" FALSE "2.x" "Needed to provide synching applications from KDE PIM applications. Necessary to compile kitchensync and other PIM applications.")

     if(OPENSYNC_FOUND AND GLIB2_FOUND)
       set(KITCHENSYNC_SUPPORTED TRUE)
       add_definitions(-DKITCHENSYNC_SUPPORTED)
       message(STATUS "KitchenSync support enabled.")
       macro_optional_add_subdirectory(kitchensync)
     else(OPENSYNC_FOUND AND GLIB2_FOUND)
       set(KITCHENSYNC_SUPPORTED FALSE)
       if(NOT OPENSYNC_FOUND)
         message(STATUS "KitchenSync support disabled (OpenSync not found).")
       endif(NOT OPENSYNC_FOUND)
       if(NOT GLIB2_FOUND)
         message(STATUS "KitchenSync support disabled (GLIB not found).")
       endif(NOT GLIB2_FOUND)
     endif(OPENSYNC_FOUND AND GLIB2_FOUND)
  else(NOT WIN32)
     set(KITCHENSYNC_SUPPORTED FALSE)
     message(STATUS "KitchenSync support DISABLED (Windows target).")
  endif(NOT WIN32)

  add_subdirectory(kresources)
  macro_optional_add_subdirectory(ktnef)
  macro_optional_add_subdirectory(kontact)

  if(QGPGME_FOUND)
     add_subdirectory(wizards)
  endif(QGPGME_FOUND)

  add_subdirectory(cmake)

  # maildir access library
  add_subdirectory(maildir)

  add_subdirectory(plugins)

endif( ONLY_KLEO )

macro_display_feature_log()
