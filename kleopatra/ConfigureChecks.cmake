# assuan configure checks
include(CheckFunctionExists)

macro_optional_find_package(Assuan2)

set( USABLE_ASSUAN_FOUND false )

if ( ASSUAN2_FOUND )

  set( CMAKE_REQUIRED_INCLUDES ${ASSUAN2_INCLUDES} )

  if ( ASSUAN2_FOUND )
    set( CMAKE_REQUIRED_LIBRARIES ${ASSUAN2_LIBRARIES} )
    set( USABLE_ASSUAN_FOUND true )
  endif( ASSUAN2_FOUND )

  # TODO: this workaround will be removed as soon as we find better solution
  if(MINGW)
    set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${KDEWIN32_INCLUDE_DIR}/mingw)
  elseif(MSVC)
    set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${KDEWIN32_INCLUDE_DIR}/msvc)
  endif(MINGW)

endif( ASSUAN2_FOUND )

if ( USABLE_ASSUAN_FOUND )
  # check if assuan.h can be compiled standalone (it couldn't, on
  # Windows, until recently, because of a HAVE_W32_SYSTEM #ifdef in
  # there)
  check_cxx_source_compiles( "
       #include <assuan.h>
       int main() {
           return 1;
       }
       "
       USABLE_ASSUAN_FOUND )
endif( USABLE_ASSUAN_FOUND )

if ( USABLE_ASSUAN_FOUND )

  # check whether assuan and gpgme may be linked to simultaneously
  check_function_exists( "assuan_get_pointer" USABLE_ASSUAN_FOUND )

endif( USABLE_ASSUAN_FOUND )

if ( USABLE_ASSUAN_FOUND )

  # check if gpg-error already has GPG_ERR_SOURCE_KLEO
  check_cxx_source_compiles("
        #include <gpg-error.h>
        static gpg_err_source_t src = GPG_ERR_SOURCE_KLEO;
        int main() { return 0; }
        "
    HAVE_GPG_ERR_SOURCE_KLEO )

endif ( USABLE_ASSUAN_FOUND )

if ( USABLE_ASSUAN_FOUND )
  message( STATUS "Usable assuan found for Kleopatra" )
else ( USABLE_ASSUAN_FOUND )
  message( STATUS "NO usable assuan found for Kleopatra" )
endif ( USABLE_ASSUAN_FOUND )

OPTION( BUILD_libkleopatraclient "Build directory kleopatra/libkleopatraclient" ${USABLE_ASSUAN_FOUND} )

if ( NOT USABLE_ASSUAN_FOUND )
  set( BUILD_libkleopatraclient false )
endif ( NOT USABLE_ASSUAN_FOUND )

macro_bool_to_01( USABLE_ASSUAN_FOUND  HAVE_USABLE_ASSUAN )
macro_bool_to_01( USABLE_ASSUAN_FOUND  HAVE_KLEOPATRACLIENT_LIBRARY )

set(CMAKE_REQUIRED_INCLUDES)
set(CMAKE_REQUIRED_LIBRARIES)
