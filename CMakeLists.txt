project(kdepim)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

# search packages used by KDE
find_package (KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

find_package(KdepimLibs REQUIRED)
find_package(Kode REQUIRED)
find_package(ZLIB REQUIRED)

if(Q_WS_X11)
   macro_log_feature(X11_Xss_LIB "X Development" "X Window Development Screen Saver Library" "http://xorg.freedesktop.org" TRUE "" "Required to build the KTimeTracker Program")
endif(Q_WS_X11)

#we require the gpgme package for Unix.
#we make gpgme optional for non-Unix since it does not exist yet.
if(UNIX)
  find_package(Gpgme REQUIRED)
else(UNIX)
  macro_optional_find_package(Gpgme)
  macro_log_feature(GPGME_FOUND "gpgme" "GnuPG Made Easy Development Libraries" "http://www.gnupg.org/related_software/gpgme" FALSE "0.4.5 or greater" "Needed to provide GNU Privacy Guard support in KDE PIM applications. Necessary to compile many PIM application, including KMail.")
endif(UNIX)
macro_bool_to_01(GPGME_FOUND HAVE_GPGME)

macro_optional_find_package(Sasl2)
macro_log_feature(SASL2_FOUND "cyrus-sasl" "Cyrus SASL API" "http://asg.web.cmu.edu/sasl/sasl-library.html" FALSE "" "Needed to support authentication of logins")

macro_optional_find_package(Boost)
macro_log_feature(BOOST_FOUND "boost" "Boost C++ Development Libraries" "http://www.boost.org" FALSE "" "Needed to support the full text indexing feature in KMail")

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


add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)
add_definitions (-D_FILE_OFFSET_BITS=64 -D_LARGE_FILES) # for off_t
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
link_directories (${KDE4_LIB_DIR})
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

set(libkdgantt_SRCS
           ${CMAKE_SOURCE_DIR}/kdgantt/KDGanttView.cpp                   
	   ${CMAKE_SOURCE_DIR}/kdgantt/KDGanttViewEventItem.cpp
           ${CMAKE_SOURCE_DIR}/kdgantt/KDGanttViewItem.cpp     
           ${CMAKE_SOURCE_DIR}/kdgantt/KDGanttViewItemDrag.cpp     
           ${CMAKE_SOURCE_DIR}/kdgantt/KDGanttViewSubwidgets.cpp   
           ${CMAKE_SOURCE_DIR}/kdgantt/KDGanttViewSummaryItem.cpp  
           ${CMAKE_SOURCE_DIR}/kdgantt/KDGanttViewTaskItem.cpp     
           ${CMAKE_SOURCE_DIR}/kdgantt/KDGanttViewTaskLink.cpp     
           ${CMAKE_SOURCE_DIR}/kdgantt/KDGanttViewTaskLinkGroup.cpp    
           ${CMAKE_SOURCE_DIR}/kdgantt/KDGanttMinimizeSplitter.cpp     
           ${CMAKE_SOURCE_DIR}/kdgantt/KDGanttSemiSizingControl.cpp        
           ${CMAKE_SOURCE_DIR}/kdgantt/KDGanttSizingControl.cpp        
           ${CMAKE_SOURCE_DIR}/kdgantt/KDGanttXMLTools.cpp         
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
add_subdirectory(akregator)
add_subdirectory(doc)

if(GPGME_FOUND)
   add_subdirectory(gpgmepp)
endif(GPGME_FOUND)

add_subdirectory(indexlib)
add_subdirectory(knode)
add_subdirectory(kabc)
add_subdirectory(kaddressbook)

add_subdirectory(kalarm)

if (Q_WS_X11)
   add_subdirectory(ktimetracker)
endif (Q_WS_X11)

add_subdirectory(kdgantt)
add_subdirectory(kfile-plugins)
add_subdirectory(kioslave)

if(GPGME_FOUND)
   add_subdirectory(kmail)
endif(GPGME_FOUND)

add_subdirectory(kmailcvt)
add_subdirectory(knotes)
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
add_subdirectory(mailtransport)

macro_display_feature_log()
