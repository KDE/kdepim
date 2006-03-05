kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kio_mbox_PART_SRCS 
   mbox.cc 
   mboxfile.cc 
   readmbox.cc 
   stat.cc 
   urlinfo.cc )

kde4_automoc(${kio_mbox_PART_SRCS})

kde4_add_plugin(kio_mbox ${kio_mbox_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kio_mbox )

target_link_libraries(kio_mbox  ${QT_AND_KDECORE_LIBS} kio )

install_targets(${LIB_INSTALL_DIR}/kde4 kio_mbox )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES mbox.protocol )

kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= $(all_includes)
#
######## Files
#
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = kio_mbox.la
#
#kio_mbox_la_SOURCES = \
#	mbox.cc \
#	mboxfile.cc \
#	readmbox.cc \
#	stat.cc \
#	urlinfo.cc
#kio_mbox_la_LIBADD  = $(LIB_KIO)
#kio_mbox_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module $(KDE_PLUGIN)
#
#noinst_HEADERS = \
#	mbox.h \
#	mboxfile.h \
#	readmbox.h \
#	stat.h \
#	urlinfo.h
#
#kdelnk_DATA = mbox.protocol
#kdelnkdir = $(kde_servicesdir)
#
#
#include $(top_srcdir)/admin/Doxyfile.am
#
