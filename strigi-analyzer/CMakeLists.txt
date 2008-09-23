macro_bool_to_01(STRIGI_NEEDS_SIGNED_CHAR STRIGI_NEEDS_SIGNED_CHAR_01)

configure_file(config-strigi.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-strigi.h )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory( vcf )
add_subdirectory( ics )
add_subdirectory( rfc822 )
