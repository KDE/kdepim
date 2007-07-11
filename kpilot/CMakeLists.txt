if (NOT PROJECT_NAME)
	MESSAGE(STATUS "Building a standalone KPilot")
	SET(KPILOT_STANDALONE YES)
	project(kpilot)
endif (NOT PROJECT_NAME)

if(EXISTS ${CMAKE_SOURCE_DIR}/CMakeOptions.txt)
	include(${CMAKE_SOURCE_DIR}/CMakeOptions.txt)
endif(EXISTS ${CMAKE_SOURCE_DIR}/CMakeOptions.txt)

if (KPILOT_STANDALONE)
	find_package(KDE4 REQUIRED) # find and setup KDE4 for this project
	include(KDE4Defaults)
	include(MacroLibrary)
	include(MacroOptionalAddSubdirectory)

	find_package(KdepimLibs REQUIRED)

	# Disallow in-source build
	STRING(COMPARE EQUAL "${kpilot_SOURCE_DIR}" "${kpilot_BINARY_DIR}" insource)
	if(insource)
		MESSAGE(FATAL_ERROR 
			"KPilot requires an out of source build. Please create a separate build directory and run 'cmake path_to_kpilot [options]' there."
		)
	endif(insource)
endif (KPILOT_STANDALONE)

# Search our own cmake modules path first
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH}) 

macro_optional_find_package(Pilotlink)
macro_optional_find_package(Mal)
macro_log_feature(PILOTLINK_FOUND "pilot-link" "Pilot-link Development Library" "http://www.pilot-link.org/" FALSE "0.12" "Needed for KPilot.")
macro_log_feature(MAL_FOUND "libmal" "libmal Development Library" "http://jasonday.home.att.net/code/libmal" FALSE "0.40" "Needed for KPilot's Avantgo conduit.")

add_definitions(
	${QT_DEFINITIONS}
	${KDE4_DEFINITIONS}
	-DQT_THREAD_SUPPORT
)

include(ConfigureChecks.cmake)

# now create config headers
configure_file(config-kpilot.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kpilot.h )

STRING(COMPARE EQUAL "${CMAKE_BUILD_TYPE}" "debug" builddebug)
STRING(COMPARE EQUAL "${CMAKE_BUILD_TYPE}" "debugfull" builddebugfull)
if (NOT builddebug AND NOT builddebugfull)
	add_definitions(-DNDEBUG)
endif(NOT builddebug AND NOT builddebugfull)


# tell cmake where to search for Qt/KDE headers:
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${KDE4_INCLUDES} 
	${PILOTLINK_INCLUDE_DIR} 
	${QT_INCLUDE_DIR} 
	${QT_QTDBUS_INCLUDE_DIR})

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

# Only build stuff if p-l is found
if (PILOTLINK_FOUND)
# tell cmake to process CMakeLists.txt in that subdirectory
add_subdirectory(lib)
add_subdirectory(kpilot)
add_subdirectory(conduits)

STRING(COMPARE EQUAL ${KDE4_BUILD_TESTS} "ON" buildtests)
if (buildtests)
	MESSAGE(STATUS "KPILOT: Test suite enabled.")
	enable_testing()
	add_subdirectory(tests)
else (buildtests)
	MESSAGE(STATUS "KPILOT: Test suite disabled.")
endif(buildtests)

if (builddebug)
	MESSAGE(STATUS "KPILOT: Debug build selected.")
else(builddebug)
	MESSAGE(STATUS "KPILOT: Normal build selected.")
endif(builddebug)
endif (PILOTLINK_FOUND)


