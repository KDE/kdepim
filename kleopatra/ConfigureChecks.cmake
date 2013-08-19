# assuan configure checks
include(CheckFunctionExists)

find_package(Assuan2)
if ( ASSUAN2_FOUND )
  set ( ASSUAN_SUFFIX "2" )
else ( ASSUAN2_FOUND )
  find_package(Assuan)
  set ( ASSUAN_SUFFIX )
endif ( ASSUAN2_FOUND )

set( USABLE_ASSUAN_FOUND false )

if ( ASSUAN${ASSUAN_SUFFIX}_FOUND )

  set( CMAKE_REQUIRED_INCLUDES ${ASSUAN${ASSUAN_SUFFIX}_INCLUDES} )

  if ( ASSUAN2_FOUND )
    set( CMAKE_REQUIRED_LIBRARIES ${ASSUAN2_LIBRARIES} )
    set( USABLE_ASSUAN_FOUND true )
  elseif ( WIN32 AND ASSUAN_VANILLA_FOUND )
    set( CMAKE_REQUIRED_LIBRARIES ${ASSUAN_VANILLA_LIBRARIES} )
    set( USABLE_ASSUAN_FOUND true )
  elseif( NOT WIN32 AND ASSUAN_PTHREAD_FOUND )
    set( CMAKE_REQUIRED_LIBRARIES ${ASSUAN_PTHREAD_LIBRARIES} )
    set( USABLE_ASSUAN_FOUND true )
  endif( ASSUAN2_FOUND )

  # TODO: this workaround will be removed as soon as we find better solution
  if(MINGW)
    set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${KDEWIN32_INCLUDE_DIR}/mingw)
  elseif(MSVC)
    set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${KDEWIN32_INCLUDE_DIR}/msvc)
  endif(MINGW)

endif( ASSUAN${ASSUAN_SUFFIX}_FOUND )

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

if ( USABLE_ASSUAN_FOUND AND NOT ASSUAN2_FOUND )

  # check if assuan has assuan_fd_t
  check_cxx_source_compiles("
        #include <assuan.h>
        int main() {
            assuan_fd_t fd = ASSUAN_INVALID_FD;
            return fd ? 1 : 0 ;
        }
        "
    HAVE_ASSUAN_FD_T )

  if ( WIN32 AND NOT HAVE_ASSUAN_FD_T )
    set( USABLE_ASSUAN_FOUND false )
  endif ( WIN32 AND NOT HAVE_ASSUAN_FD_T )

  # check if assuan has assuan_inquire_ext, old style
  check_function_exists( "assuan_inquire_ext" HAVE_ASSUAN_INQUIRE_EXT )

  if ( NOT HAVE_ASSUAN_INQUIRE_EXT )
    set( USABLE_ASSUAN_FOUND false )
  endif( NOT HAVE_ASSUAN_INQUIRE_EXT )

  # check if assuan has new-style assuan_inquire_ext:
  check_cxx_source_compiles("
        #include <assuan.h>
        static int handler( void *, int, unsigned char*, size_t ) { return 0; }
        int main() {
            assuan_context_t ctx = 0;
            const size_t maxSize = 0U;
            assuan_error_t err = assuan_inquire_ext( ctx, \"FOO\", maxSize, handler, (void*)0 );
            return err ? 1 : 0 ;
        }
        "
    HAVE_NEW_STYLE_ASSUAN_INQUIRE_EXT )

endif( USABLE_ASSUAN_FOUND AND NOT ASSUAN2_FOUND )

if ( USABLE_ASSUAN_FOUND )

  # check if gpg-error already has GPG_ERR_SOURCE_KLEO
  check_cxx_source_compiles("
        #include <gpg-error.h>
        static gpg_err_source_t src = GPG_ERR_SOURCE_KLEO;
        int main() { return 0; }
        "
    HAVE_GPG_ERR_SOURCE_KLEO )

endif ( USABLE_ASSUAN_FOUND )

if ( USABLE_ASSUAN_FOUND AND NOT ASSUAN2_FOUND )

  # check if assuan has assuan_sock_get_nonce (via assuan_sock_nonce_t)
  # function_exists runs into linking errors - libassuan is static,
  # and assuan_sock_get_nonce drags in stuff that needs linking
  # against winsock2.
  check_cxx_source_compiles("
        #include <assuan.h>
        static assuan_sock_nonce_t nonce;
        int main() { return 0; }
        "
    HAVE_ASSUAN_SOCK_GET_NONCE )

  if ( WIN32 AND NOT HAVE_ASSUAN_SOCK_GET_NONCE )
    set( USABLE_ASSUAN_FOUND false )
  endif ( WIN32 AND NOT HAVE_ASSUAN_SOCK_GET_NONCE )  

endif ( USABLE_ASSUAN_FOUND AND NOT ASSUAN2_FOUND )

if ( USABLE_ASSUAN_FOUND )
  message( STATUS "Usable assuan found for Kleopatra" )
else ( USABLE_ASSUAN_FOUND )
  message( STATUS "NO usable assuan found for Kleopatra" )
endif ( USABLE_ASSUAN_FOUND )

if ( NOT ASSUAN2_FOUND )

#
# Check that libassuan (which is built statically) can be linked into a DSO
# (e.g. on amd64, this requires it to be compiled with -fPIC).
#

set ( ASSUAN_LINKABLE_TO_DSO false )

endif ( NOT ASSUAN2_FOUND )

OPTION( BUILD_libkleopatraclient "Build directory kleopatra/libkleopatraclient" ${USABLE_ASSUAN_FOUND} )

if ( NOT USABLE_ASSUAN_FOUND )
  set( BUILD_libkleopatraclient false )
endif ( NOT USABLE_ASSUAN_FOUND )

if ( BUILD_libkleopatraclient AND NOT ASSUAN2_FOUND )

  message( STATUS "Checking whether libassuan can be linked against from DSO's" )

  set ( YUP TRUE )
  if ( YUP )
    set ( ASSUAN_LINKABLE_TO_DSO true )
    message( STATUS "--> Assuming that it can. If compilation of libkleopatraclient fails on AMD64, check that libassuan is compiled with -fPIC and try again. Otherwise, pass -DBUILD_libkleopatraclient=OFF." )
  else ( YUP )
  # TODO: make this one executed at configure time, so the check below works:
  add_library( dso_with_assuan_check SHARED ${CMAKE_SOURCE_DIR}/kleopatra/dso_with_assuan_check.c )

  set( CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} dso_with_assuan_check )
  check_cxx_source_compiles( "int main() { return 0; }" ASSUAN_LINKABLE_TO_DSO )

  if ( ASSUAN_LINKABLE_TO_DSO )
    message( STATUS "Usable assuan found for libkleopatraclient" )
  else ( ASSUAN_LINKABLE_TO_DSO )
    message( STATUS "NO usable assuan found for libkleopatraclient - if this is AMD64, check that libassuan is compiled with -fPIC" )
  endif ( ASSUAN_LINKABLE_TO_DSO )
  endif ( YUP )

endif ( BUILD_libkleopatraclient AND NOT ASSUAN2_FOUND )

macro_bool_to_01( USABLE_ASSUAN_FOUND  HAVE_USABLE_ASSUAN )
macro_bool_to_01( ASSUAN2_FOUND HAVE_ASSUAN2 )
if ( ASSUAN2_FOUND )
macro_bool_to_01( USABLE_ASSUAN_FOUND  HAVE_KLEOPATRACLIENT_LIBRARY )
else ( ASSUAN2_FOUND )
macro_bool_to_01( ASSUAN_LINKABLE_TO_DSO HAVE_KLEOPATRACLIENT_LIBRARY )
endif ( ASSUAN2_FOUND )

set(CMAKE_REQUIRED_INCLUDES)
set(CMAKE_REQUIRED_LIBRARIES)
