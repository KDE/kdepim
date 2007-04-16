project(kpilot)

if(EXISTS ${CMAKE_SOURCE_DIR}/CMakeOptions.txt)
	include(${CMAKE_SOURCE_DIR}/CMakeOptions.txt)
else(EXISTS ${CMAKE_SOURCE_DIR}/CMakeOptions.txt)
	message(FATAL_ERROR "CMakeOptions.txt not found! Run configure first.")
endif(EXISTS ${CMAKE_SOURCE_DIR}/CMakeOptions.txt)

# Search our own cmake modules path first
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

# Need 2.4.3 for its KDE3 support
# Need 2.4.4 for its fixed KDE3 support
cmake_minimum_required(VERSION 2.4.4)

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# Disallow in-source build
STRING(COMPARE EQUAL "${kpilot_SOURCE_DIR}" "${kpilot_BINARY_DIR}" insource)
if(insource)
	MESSAGE(FATAL_ERROR 
		"KPilot requires an out of source build. Please create a separate build
directory and run 'cmake path_to_kpilot [options]' there."
	)
endif(insource)

find_package(Qt3 REQUIRED) # find and setup Qt3 for this project
find_package(KDE3 REQUIRED) # find and setup KDE3 for this project
find_package(Pilotlink REQUIRED)
find_package(Mal) # see if mal is available, but it's not required

add_definitions(
	${QT_DEFINITIONS}
	${KDE3_DEFINITIONS}
	-DQT_THREAD_SUPPORT
)

STRING(COMPARE EQUAL "${CMAKE_BUILD_TYPE}" "debug" builddebug)
if (NOT builddebug)
	add_definitions(-DNDEBUG)
endif(NOT builddebug)


# Get the kde3 dir. This is a bit tricky, i'm not sure how well
# this works on other systems.
STRING(REPLACE "/lib" "" KDE3_DIR ${KDE3_LIB_DIR})

# TODO: INSTALL PREFIX. RIGHT NOW EVERYTHING IS INSTALLED IN $KDEDIR
if(NOT CMAKE_INSTALL_PREFIX)
	set(CMAKE_INSTALL_PREFIX ${KDE3_DIR})
endif(NOT CMAKE_INSTALL_PREFIX)
set(KDE3_KCFG_DIR ${CMAKE_INSTALL_PREFIX}/share/config.kcfg)
set(KDE3_SERVICETYPES_DIR ${CMAKE_INSTALL_PREFIX}/share/servicetypes)
set(KDE3_SERVICES_DIR ${CMAKE_INSTALL_PREFIX}/share/services)
set(KDE3_XDG_APPS_DIR ${CMAKE_INSTALL_PREFIX}/share/applications/kde)
set(KDE3_LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib)
set(KDE3_PLUGIN_INSTALL_DIR ${KDE3_LIB_INSTALL_DIR}/kde3)

# tell cmake where to search for libraries:
link_directories(${KDE3_LIB_DIR})

# tell cmake where to search for Qt/KDE headers:
include_directories(${PILOTLINK_INCLUDE_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR})

# include custom macros
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/modules/KPilotCustom.cmake)

# tell cmake to process CMakeLists.txt in that subdirectory
add_subdirectory(lib)
add_subdirectory(kpilot)
add_subdirectory(conduits)


STRING(COMPARE EQUAL "${ENABLE_TESTS}" "YES" buildtests)
if (buildtests)
	MESSAGE(STATUS "BUILD: Test suite enabled.")
	enable_testing()
	add_subdirectory(tests)
else (buildtests)
	MESSAGE(STATUS "BUILD: Test suite disabled.")
endif(buildtests)

if (builddebug)
	MESSAGE(STATUS "BUILD: Debug build selected.")
else(builddebug)
	MESSAGE(STATUS "BUILD: Normal build selected.")
endif(builddebug)

MESSAGE(STATUS "BUILD: Install prefix set to ${CMAKE_INSTALL_PREFIX} .")

