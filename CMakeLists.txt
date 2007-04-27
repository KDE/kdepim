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

if(Q_WS_X11)
   macro_log_feature(X11_Xss_LIB "libXss" "X Window Screen Saver Development Library" "http://xorg.freedesktop.org" FALSE "" "Required to build the KTimeTracker Program")
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

macro_optional_find_package(Opensync)
#macro_log_feature for Opensync is performed in the FindOpensync.cmake

# is this really required here ?
if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)


add_definitions (-D_FILE_OFFSET_BITS=64 -D_LARGE_FILES) # for off_t
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${QT_QTDBUS_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/libkdepim/)


include (ConfigureChecks.cmake)
include (gpgmepp/ConfigureChecks.cmake)
configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)


set(libkabcommon_SRCS
  ${CMAKE_SOURCE_DIR}/kaddressbook/common/kabprefs.cpp
  ${CMAKE_SOURCE_DIR}/kaddressbook/common/locationmap.cpp
  ${CMAKE_SOURCE_DIR}/kaddressbook/common/filter.cpp
)

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
add_subdirectory(libkholidays_ng)

if(GPGME_FOUND)
   add_subdirectory(libkleo)
endif(GPGME_FOUND)

add_subdirectory(libkpgp)

if(GPGME_FOUND)
   add_subdirectory(kleopatra)
endif(GPGME_FOUND)
add_subdirectory(libkpimidentities)

add_subdirectory(libksieve)
add_subdirectory(mimelib)
macro_optional_add_subdirectory(akregator)
macro_optional_add_subdirectory(doc)

if(GPGME_FOUND)
   add_subdirectory(gpgmepp)
endif(GPGME_FOUND)

if(OPENSYNC_FOUND AND OSENGINE_FOUND)
   macro_optional_add_subdirectory(kitchensync)
endif(OPENSYNC_FOUND AND OSENGINE_FOUND)
macro_optional_add_subdirectory(knode)
macro_optional_add_subdirectory(kaddressbook)

macro_optional_add_subdirectory(kalarm)

if (Q_WS_X11)
   macro_optional_add_subdirectory(ktimetracker)
endif (Q_WS_X11)

add_subdirectory(kdgantt1)
MESSAGE(STATUS "Remember to porting kfile-plugins")
add_subdirectory(kfile-plugins)
add_subdirectory(kioslave)

if(GPGME_FOUND)
   macro_optional_add_subdirectory(kmail)
endif(GPGME_FOUND)

macro_optional_add_subdirectory(kmailcvt)
macro_optional_add_subdirectory(knotes)
add_subdirectory(console)
add_subdirectory(kontact)

if(GPGME_FOUND)
   add_subdirectory(korganizer)
endif(GPGME_FOUND)

add_subdirectory(korn)
add_subdirectory(kresources)
add_subdirectory(ktnef)
MESSAGE(STATUS "disabled networkstatus")
#add_subdirectory(networkstatus)

if(GPGME_FOUND)
   add_subdirectory(plugins)
   add_subdirectory(qgpgme)
   add_subdirectory(wizards)
endif(GPGME_FOUND)

add_subdirectory(cmake)

#mailtransport move to kdepimlibs is not complete.
add_subdirectory(mailtransport)

macro_display_feature_log()
