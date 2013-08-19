
if(NOT MSVC)
  set(HAVE_C99_INITIALIZERS 1)
endif()

configure_file (config-chiasmus.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-chiasmus.h )
