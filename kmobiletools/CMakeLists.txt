project(kmobiletools)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/akonadi )


add_definitions(-DQT3_SUPPORT)
add_definitions(-DQT3_SUPPORT_WARNINGS)
OPTION(KMOBILETOOLS_USE_CRASHHANDLER "Use internal CrashHandler Manager" OFF)
OPTION(KMOBILETOOLS_ENABLE_TESTS "Enable compilation of testing applications" OFF)



message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(KIOSLAVES_DIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(TEST_DIR)")

check_include_files(gammu/gammu.h HAVE_GAMMU)

configure_file( config-kmobiletools.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kmobiletools.h )

add_subdirectory( kmobiletools )
if ( KMOBILETOOLS_ENABLE_TESTS )
    add_subdirectory ( tests )
    message ( "-- Enabling KMobileTools tests compilation" )
endif ( KMOBILETOOLS_ENABLE_TESTS )

#add_subdirectory( kbluetoothpairingwizard )
#add_subdirectory( kmtsetup )
