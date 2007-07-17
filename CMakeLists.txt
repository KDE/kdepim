project(kdepim)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

# search packages used by KDE
find_package (KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(MacroOptionalAddSubdirectory)

find_package(KdepimLibs REQUIRED)
find_package(Kode REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Strigi REQUIRED)

if(Q_WS_X11)
   macro_log_feature(X11_Xscreensaver_LIB "libXss" "X Window Screen Saver Development Library" "http://xorg.freedesktop.org" FALSE "" "Required to build the KTimeTracker Program")
endif(Q_WS_X11)

#we require the gpgme package for Unix.
#we make gpgme optional for non-Unix since it does not exist yet.
if(UNIX)
  find_package(Gpgme REQUIRED)
else(UNIX)
  find_package(Gpgme)
endif(UNIX)

macro_optional_find_package(Sasl2)
macro_log_feature(SASL2_FOUND "cyrus-sasl" "Cyrus SASL API" "http://asg.web.cmu.edu/sasl/sasl-library.html" FALSE "" "Needed to support authentication of logins")

macro_optional_find_package(GNOKII)
macro_log_feature(GNOKII_FOUND "gnokii" "Gnokii Development Libraries" "http://www.gnokii.org" FALSE "" "Needed to provide the ability to import and export KDE addressbook entries to/from mobile phones that are accessible via gnokii")

macro_optional_find_package(MySQL)
macro_log_feature(MYSQL_EMBEDDED_FOUND "mysql-embedded" "MySQL/Embedded Library" "http://www.mysql.org" FALSE "" "Needed for the MySQL/Embedded Akonadi backend. Sqlite will be used instead.")

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

if(GPGME_FOUND)
   macro_optional_add_subdirectory(kleopatra)
endif(GPGME_FOUND)
if(NOT WIN32)
  add_subdirectory(libkpgp)
endif(NOT WIN32)

add_subdirectory(libksieve)
add_subdirectory(mimelib)
macro_optional_add_subdirectory(kfeed)
macro_optional_add_subdirectory(akregator)
macro_optional_add_subdirectory(doc)
if(NOT WIN32)
  # Laurent knode/kmail depend against libkpgp which doesn't compile on windows system for the moment
  macro_optional_add_subdirectory(knode)
  if(GPGME_FOUND)
    macro_optional_add_subdirectory(kmail)
    add_subdirectory(plugins)
  endif(GPGME_FOUND)
endif(NOT WIN32)
macro_optional_add_subdirectory(kaddressbook)

macro_optional_add_subdirectory(kalarm)

if (Q_WS_X11)
   macro_optional_add_subdirectory(ktimetracker)
endif (Q_WS_X11)

add_subdirectory(kdgantt1)
macro_optional_add_subdirectory(strigi-analyzer)
add_subdirectory(kioslave)

macro_optional_add_subdirectory(kmailcvt)
macro_optional_add_subdirectory(knotes)
macro_optional_add_subdirectory(console)
macro_optional_add_subdirectory(kontact)

if(GPGME_FOUND)
   macro_optional_add_subdirectory(korganizer)
endif(GPGME_FOUND)

if( NOT WIN32)
 macro_optional_find_package(Opensync)
 macro_optional_add_subdirectory(kmobiletools)
 macro_optional_add_subdirectory(kpilot)
 if(OPENSYNC_FOUND AND OSENGINE_FOUND)
   macro_optional_add_subdirectory(kitchensync)
 endif(OPENSYNC_FOUND AND OSENGINE_FOUND)
endif( NOT WIN32)

macro_optional_add_subdirectory(korn)
add_subdirectory(kresources)
macro_optional_add_subdirectory(ktnef)
MESSAGE(STATUS "disabled networkstatus")
#add_subdirectory(networkstatus)

if(GPGME_FOUND)
   add_subdirectory(wizards)
endif(GPGME_FOUND)

add_subdirectory(cmake)

# maildir access library
add_subdirectory(maildir)

macro_display_feature_log()
