if(WINCE)
add_definitions(-DSTRIGI_STATIC)
endif()

if(${STRIGI_VERSION} VERSION_GREATER 0.7.2)
  set(USE_STRIGI_KMPSEARCHER_H TRUE)
endif()

macro_bool_to_01(USE_STRIGI_KMPSEARCHER_H HAVE_KMPSEARCHER_H)
macro_bool_to_01(STRIGI_NEEDS_SIGNED_CHAR STRIGI_NEEDS_SIGNED_CHAR_01)

configure_file(config-strigi.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-strigi.h )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory( ctg )
add_subdirectory( vcf )
add_subdirectory( ics )
add_subdirectory( mail )
if(WINCE)
add_subdirectory( strigidaemon_static )
endif()
