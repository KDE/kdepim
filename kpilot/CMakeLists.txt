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
macro_optional_find_package(QCA2)
macro_log_feature(PILOTLINK_FOUND "pilot-link" "Pilot-link Development Library" "http://www.pilot-link.org/" FALSE "0.12" "Needed for KPilot.")
macro_log_feature(QCA2_FOUND "QCA2" "Qt Cryptographic Architecture" "http://delta.affinix.com/qca" FALSE "2.0.0" "Needed for KPilot's keyringconduit")

add_definitions(
	${QT_DEFINITIONS}
	${KDE4_DEFINITIONS}
	-DQT_THREAD_SUPPORT
)

include(ConfigureChecks.cmake)

# now create config headers
string(COMPARE EQUAL ${KDE4_BUILD_TESTS} "ON" buildtests)
macro_bool_to_01(buildtests HAVE_TESTS_ENABLED)
configure_file(config-kpilot.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kpilot.h )


# tell cmake where to search for Qt/KDE headers:
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${QT_INCLUDE_DIR}
	${QT_QTDBUS_INCLUDE_DIR}
	${KDE4_INCLUDE_DIR}
)

# Only build stuff if p-l is found
if (PILOTLINK_FOUND)
	include_directories(${PILOTLINK_INCLUDE_DIR})
	# tell cmake to process CMakeLists.txt in that subdirectory
	add_subdirectory(lib)
	add_subdirectory(kpilot)
	add_subdirectory(conduits)

	if (buildtests)
		MESSAGE(STATUS "KPILOT: Test suite enabled.")
		enable_testing()
		add_subdirectory(tests)
	else (buildtests)
		MESSAGE(STATUS "KPILOT: Test suite disabled.")
	endif(buildtests)
endif (PILOTLINK_FOUND)


