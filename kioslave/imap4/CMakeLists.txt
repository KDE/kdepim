kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libkmime ${CMAKE_SOURCE_DIR}/libemailfunctions ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kio_imap4_PART_SRCS 
   imapcommand.cc 
   imaplist.cc 
   mailaddress.cc 
   mimeheader.cc 
   rfcdecoder.cc 
   imap4.cc 
   imapinfo.cc 
   imapparser.cc 
   mailheader.cc 
   mimehdrline.cc 
   mimeio.cc )

kde4_automoc(${kio_imap4_PART_SRCS})

kde4_add_plugin(kio_imap4 ${kio_imap4_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kio_imap4 )

target_link_libraries(kio_imap4  ${QT_AND_KDECORE_LIBS} kio kmime emailfunctions )

install_targets(${LIB_INSTALL_DIR}/kde4 kio_imap4 )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES imap4.protocol imaps.protocol )

kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= -I$(top_srcdir)/libkmime \
#    -I$(srcdir)/.. $(SSL_INCLUDES) \
#    -I$(top_srcdir)/libemailfunctions \
#    $(all_includes)
#
######## Files
#
#kde_module_LTLIBRARIES = kio_imap4.la
#
#kio_imap4_la_SOURCES = 	imapcommand.cc imaplist.cc mailaddress.cc \
#  mimeheader.cc rfcdecoder.cc imap4.cc imapinfo.cc imapparser.cc mailheader.cc \
#  mimehdrline.cc  mimeio.cc
#kio_imap4_la_LIBADD  = $(LIB_KIO) $(SASL2_LIBS) ../../libkmime/libkmime.la \
#  ../../libemailfunctions/libemailfunctions.la
#kio_imap4_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module $(KDE_PLUGIN)
#
#noinst_HEADERS = imap4.h
#EXTRA_DIST = README
#
#kdelnk_DATA = imap4.protocol imaps.protocol
#kdelnkdir = $(kde_servicesdir)
#
#messages:
#	$(XGETTEXT) *.cc -o $(podir)/kio_imap4.pot
#
#include $(top_srcdir)/admin/Doxyfile.am
