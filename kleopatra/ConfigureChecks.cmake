# assuan configure checks
include(CheckFunctionExists)

set(CMAKE_REQUIRED_INCLUDES ${GPGME_INCLUDES}) ### ASSUAN_INCLUDES
set(CMAKE_REQUIRED_LIBRARIES ${GPGME_LIBRARIES} -lassuan -lgpg-error) ### ASSUAN_LIBRARIES

# check whether assuan and gpgme may be linked to simultaneously
check_function_exists( "assuan_get_pointer" ASSUAN_AND_GPGME_IN_SAME_PROCESS )

if ( ASSUAN_AND_GPGME_IN_SAME_PROCESS )

  # check if assuan has assuan_fd_t
  check_cxx_source_compiles("
        #include <assuan.h>
        int main() {
            assuan_fd_t fd = ASSUAN_INVALID_FD;
            return fd ? 1 : 0 ;
        }
        "
    HAVE_ASSUAN_FD_T )

  # check if assuan has assuan_inquire_ext, old style
  check_function_exists( "assuan_inquire_ext" HAVE_ASSUAN_INQUIRE_EXT )

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

endif ( ASSUAN_AND_GPGME_IN_SAME_PROCESS )

set( USABLE_ASSUAN_FOUND ${ASSUAN_AND_GPGME_IN_SAME_PROCESS} )
if ( USABLE_ASSUAN_FOUND AND HAVE_ASSUAN_INQUIRE_EXT )
   set( USABLE_ASSUAN_FOUND true )
endif( USABLE_ASSUAN_FOUND AND HAVE_ASSUAN_INQUIRE_EXT )
if ( WIN32 AND NOT HAVE_ASSUAN_FD_T )
   set( USABLE_ASSUAN_FOUND false )
endif ( WIN32 AND NOT HAVE_ASSUAN_FD_T )

macro_bool_to_01( USABLE_ASSUAN_FOUND  HAVE_USABLE_ASSUAN )

set(CMAKE_REQUIRED_INCLUDES)
set(CMAKE_REQUIRED_LIBRARIES)
