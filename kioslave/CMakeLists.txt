kde4_header()

add_subdirectory( imap4 ) 
add_subdirectory( mbox ) 

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(SIEVE_SUBDIR)")
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_create_doxygen_docs( REFERENCES kioslaves/imap4 kioslaves/mbox  )

kde4_footer()



#original Makefile.am contents follow:

### $Id: Makefile.am 427821 2005-06-21 23:40:49Z adridg $
#
#if compile_kio_sieve
#  SIEVE_SUBDIR = sieve
#endif
#
#SUBDIRS = imap4 mbox $(SIEVE_SUBDIR)
#
#DOXYGEN_REFERENCES = kioslaves/imap4 kioslaves/mbox
#include $(top_srcdir)/admin/Doxyfile.am
#
