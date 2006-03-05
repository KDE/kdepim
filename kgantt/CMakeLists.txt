kde4_header()

add_subdirectory( kgantt ) 
add_subdirectory( pixmaps ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = kgantt pixmaps
#
#EXTRA_DIST = AUTHORS COPYING ChangeLog INSTALL README TODO 
#
#include $(top_srcdir)/admin/Doxyfile.am
