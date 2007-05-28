project(kmobiletools)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DQT3_SUPPORT)
add_definitions(-DQT3_SUPPORT_WARNINGS)
OPTION(USE_CRASHHANDLER "Use internal CrashHandler Manager" OFF)



message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(KONTACT_PLUGIN_SUBDIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(KIOSLAVES_DIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(TEST_DIR)")

check_include_files(gammu/gammu.h HAVE_GAMMU)

configure_file( config-kmobiletools.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kmobiletools.h )

add_subdirectory( kmobiletools )
#add_subdirectory( kbluetoothpairingwizard )
#add_subdirectory( kmtsetup )

########### install files ###############




#original Makefile.am contents follow:

#KDE_CXXFLAGS = -UQT_NO_ASCII_CAST
#KDE_OPTIONS = nofinal
#if have_kontact
#    KONTACT_PLUGIN_SUBDIR = kmobiletools-kontact-plugin
#endif
#if compile_kioslaves
#    KIOSLAVES_DIR = mobile-kioslaves
#endif
#
#if compile_tests
#   TESTS_DIR = tests
#endif
#
#SUBDIRS = kmobiletools $(KONTACT_PLUGIN_SUBDIR) kbluetoothpairingwizard kmtsetup $(KIOSLAVES_DIR) $(TEST_DIR)
## mobile-kioslaves
## $(KONTACT_PLUGIN_SUBDIR) $(KSYNC_PLUGIN_SUBDIR) kmobiletools_singleDeviceTest mobile-kioslaves
#
