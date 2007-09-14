project(kdepim)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

# search packages used by KDE
find_package (KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(MacroOptionalAddSubdirectory)

if ( ONLY_KLEO )
  find_package(KdepimLibs)
  find_package(Boost REQUIRED )
  macro_log_feature(Boost_FOUND "boost" "Boost C++ Libraries" "http://www.boost.org/" TRUE "" "Required by Akonadi and kleopatra")
  find_package(QGpgme REQUIRED)
  add_definitions (-D_FILE_OFFSET_BITS=64 -D_LARGE_FILES) # for off_t
  add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
  include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${QT_QTDBUS_INCLUDE_DIR} )
  include (kleopatra/ConfigureChecks.cmake)
  add_subdirectory(libkleo)
  FIND_PATH(TOPOLOGICAL_SORT_DIR NAMES boost/graph/topological_sort.hpp PATHS ${Boost_INCLUDE_DIRS})
  if(TOPOLOGICAL_SORT_DIR)
    macro_optional_add_subdirectory(kleopatra)
  else(TOPOLOGICAL_SORT_DIR)
    MESSAGE(STATUS "topological_sort.hpp is necessary to compile kleopatra. Please install package which contains it.")
  endif(TOPOLOGICAL_SORT_DIR)
else ( ONLY_KLEO )

find_package(KdepimLibs REQUIRED)
find_package(Kode REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Strigi REQUIRED)
find_package(Boost REQUIRED )
macro_log_feature(Boost_FOUND "boost" "Boost C++ Libraries" "http://www.boost.org/" TRUE "" "Required by Akonadi and kleopatra")

if(Q_WS_X11)
   macro_log_feature(X11_Xscreensaver_LIB "libXss" "X Window Screen Saver Development Library" "http://xorg.freedesktop.org" FALSE "" "Required for building the KTimeTracker Program")
endif(Q_WS_X11)

find_package(QGpgme)

macro_optional_find_package(GNOKII)
macro_log_feature(GNOKII_FOUND "gnokii" "Gnokii Development Libraries" "http://www.gnokii.org" FALSE "" "Needed to provide the ability to import and export KDE addressbook entries to/from mobile phones that are accessible via gnokii")

macro_optional_find_package(MySQL)
macro_log_feature(MYSQL_EMBEDDED_FOUND "mysql-embedded" "MySQL/Embedded Library" "http://www.mysql.org" FALSE "" "Needed for the MySQL/Embedded Akonadi backend. Sqlite will be used instead.")

macro_optional_find_package(Nepomuk)
macro_log_feature(Nepomuk_FOUND "Nepomuk" "Nepomuk" "http://www.kde.org" FALSE "" "For adding desktop-wide tagging support to kmail")

macro_optional_find_package(LibXslt)
macro_log_feature(XSLTPROC_EXECUTABLE "xsltproc" "xsltproc" "http://xmlsoft.org/XSLT/" FALSE "" "Needed to generate Akonadi database code")

# is this really required here ?
if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)


add_definitions (-D_FILE_OFFSET_BITS=64 -D_LARGE_FILES) # for off_t
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${QT_QTDBUS_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/libkdepim/)


include (ConfigureChecks.cmake)
include (kleopatra/ConfigureChecks.cmake)
configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

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
add_subdirectory(akonadi)
add_subdirectory(libkdepim)
add_subdirectory(libkholidays)

FIND_PATH(TOPOLOGICAL_SORT_DIR NAMES boost/graph/topological_sort.hpp PATHS ${Boost_INCLUDE_DIRS})
if(QGPGME_FOUND)
   if(TOPOLOGICAL_SORT_DIR)
      macro_optional_add_subdirectory(kleopatra)
   else(TOPOLOGICAL_SORT_DIR)
      MESSAGE(STATUS "topological_sort.hpp is necessary to compile kleopatra. Please install package which contains it.")
   endif(TOPOLOGICAL_SORT_DIR)
   add_subdirectory(libkleo)
endif(QGPGME_FOUND)

add_subdirectory(libkpgp)
add_subdirectory(libksieve)
add_subdirectory(mimelib)
macro_optional_add_subdirectory(kfeed)
macro_optional_add_subdirectory(akregator)
macro_optional_add_subdirectory(doc)
macro_optional_add_subdirectory(knode)
if(QGPGME_FOUND)
  macro_optional_add_subdirectory(kmail)
  add_subdirectory(plugins)
endif(QGPGME_FOUND)
macro_optional_add_subdirectory(kalarm)

macro_optional_add_subdirectory(kaddressbook)

if (Q_WS_X11)
   macro_optional_add_subdirectory(ktimetracker)
endif (Q_WS_X11)

add_subdirectory(kdgantt1)
macro_optional_add_subdirectory(strigi-analyzer)

macro_optional_add_subdirectory(kmailcvt)
macro_optional_add_subdirectory(knotes)
macro_optional_add_subdirectory(console)
macro_optional_add_subdirectory(kontact)

if(QGPGME_FOUND)
   macro_optional_add_subdirectory(korganizer)
endif(QGPGME_FOUND)

if(NOT WIN32)
   macro_optional_find_package(Opensync)
   #macro_log_feature() not needed because FindOpensync.cmake does so for us
   macro_optional_find_package(GLIB2)
   macro_log_feature(GLIB2_FOUND "glib2" "Low-level core library for data structure handling, portability wrappers, etc." "http://www.gtk.org" FALSE "2.x" "Needed to provide synching applications from KDE PIM applications. Necessary to compile kitchensync and other PIM applications.")
   macro_optional_add_subdirectory(kmobiletools)
   macro_optional_add_subdirectory(kpilot)
   if(OPENSYNC_FOUND AND GLIB2_FOUND)
      macro_optional_add_subdirectory(kitchensync)
   endif(OPENSYNC_FOUND AND GLIB2_FOUND)
endif(NOT WIN32)

macro_optional_add_subdirectory(korn)
add_subdirectory(kresources)
macro_optional_add_subdirectory(ktnef)
MESSAGE(STATUS "disabled networkstatus")
#add_subdirectory(networkstatus)

if(QGPGME_FOUND)
   add_subdirectory(wizards)
endif(QGPGME_FOUND)

add_subdirectory(cmake)

# maildir access library
add_subdirectory(maildir)

endif( ONLY_KLEO )

macro_display_feature_log()
