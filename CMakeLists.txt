project(kdepim)

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# search packages used by KDE
find_package(KDE4 REQUIRED)
include(KDE4Defaults)

find_package(Perl REQUIRED)
if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)


find_package(Kode REQUIRED)
macro_optional_find_package(LIBKDEPIM)
macro_optional_find_package(Sasl2)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
link_directories(${KDE4_LIB_DIR})


include (MacroLibrary)

include (ConfigureChecks.cmake)
include (gpgmepp/ConfigureChecks.cmake)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

include_directories(
  ${CMAKE_SOURCE_DIR}/libkdepim ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

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

set(libemailfunctions_SRCS 
	${CMAKE_SOURCE_DIR}/libemailfunctions/email.cpp 
	${CMAKE_SOURCE_DIR}/libemailfunctions/idmapper.cpp 
	${CMAKE_SOURCE_DIR}/libemailfunctions/kasciistringtools.cpp
	${CMAKE_SOURCE_DIR}/libemailfunctions/networkstatus.cpp
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



add_subdirectory(libakonadi)
add_subdirectory(libemailfunctions)
add_subdirectory(libkcal)
add_subdirectory(libkdepim)
add_subdirectory(libkholidays)
add_subdirectory(libkholidays_ng)
add_subdirectory(libkleo)
add_subdirectory(libkmime)
add_subdirectory(libkpgp)
add_subdirectory(kleopatra)
add_subdirectory(libkpimidentities)
add_subdirectory(libksieve)
add_subdirectory(mimelib)
add_subdirectory(libsyndication)
add_subdirectory(akregator)
add_subdirectory(doc)
add_subdirectory(gpgmepp)
add_subdirectory(indexlib)
add_subdirectory(knode)
add_subdirectory(kabc)
add_subdirectory(kaddressbook)
add_subdirectory(kalarm)
add_subdirectory(karm)
add_subdirectory(kdgantt)
add_subdirectory(kfile-plugins)
add_subdirectory(kioslave)
add_subdirectory(kmail)
add_subdirectory(kmailcvt)
add_subdirectory(knotes)
add_subdirectory(kode)
add_subdirectory(konsolekalendar)
add_subdirectory(kontact)
add_subdirectory(korganizer)
add_subdirectory(korn)
add_subdirectory(kresources)
add_subdirectory(ktnef)
add_subdirectory(networkstatus)
add_subdirectory(plugins)
add_subdirectory(qgpgme)
add_subdirectory(wizards)

add_subdirectory(cmake)
