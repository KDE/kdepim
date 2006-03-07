project(kdepim)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
# Perhaps define share/apps as ${DATA_INSTALL_DIR}
#set(CMAKE_MODULE_PATH "${KDEDIR}/share/apps/cmake/modules/ ${CMAKE_SOURCE_DIR}/cmake/modules")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# support for Dart: http://public.kitware.com/dashboard.php?name=kde
enable_testing()
include(CTest)

# generate non-quiet makefiles
set(CMAKE_VERBOSE_MAKEFILE ON)

# search packages used by KDE
find_package(KDE4 REQUIRED)
find_package(Perl REQUIRED)
find_package(ZLIB REQUIRED)

if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)


macro_optional_find_package(LIBKDEPIM)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
link_directories(${KDE4_LIB_DIR})


include (MacroAdditionalCleanFiles)
include (MacroAddFileDependencies)
include (ConfigureChecks.cmake)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

include_directories(
  ${CMAKE_SOURCE_DIR}/libkdepim ${KDE4_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(libemailfunctions_SRCS 
	${CMAKE_SOURCE_DIR}/libemailfunctions/email.cpp 
	${CMAKE_SOURCE_DIR}/libemailfunctions/idmapper.cpp 
	${CMAKE_SOURCE_DIR}/libemailfunctions/kasciistricmp.cpp
	${CMAKE_SOURCE_DIR}/libemailfunctions/kasciistringtools.cpp
	${CMAKE_SOURCE_DIR}/libemailfunctions/networkstatus.cpp
	)


add_subdirectory(libakonadi)
add_subdirectory(libemailfunctions)
#add_subdirectory(libkcal)
add_subdirectory(libkdepim)
add_subdirectory(libkholidays)
add_subdirectory(libkleo)
add_subdirectory(libkmime)
add_subdirectory(libkpgp)
add_subdirectory(kleopatra)
add_subdirectory(libkpimidentities)
add_subdirectory(libksieve)
add_subdirectory(mimelib)
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
add_subdirectory(kgantt)
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

MESSAGE(STATUS "Perhaps not install FindKDE4. It will conflict when we install with rpm/deb package")
add_subdirectory( cmake )

kde4_create_doxygen_docs(  )

