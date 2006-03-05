kde4_header()

add_subdirectory( mimetypes ) 

include_directories( ${CMAKE_SOURCE_DIR}/kmobile ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kio_mobile_PART_SRCS kio_mobile.cpp )

kde4_automoc(${kio_mobile_PART_SRCS})

kde4_add_plugin(kio_mobile ${kio_mobile_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kio_mobile )

target_link_libraries(kio_mobile  ${QT_AND_KDECORE_LIBS} kio kmobileclient )

install_targets(${LIB_INSTALL_DIR}/kde4 kio_mobile )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES mobile.protocol cellphone.protocol  	organizer.protocol pda.protocol  )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS  = mimetypes
#
#INCLUDES = -I$(top_srcdir)/kmobile -I$(top_srcdir) $(all_includes)
#
#kde_module_LTLIBRARIES = kio_mobile.la
#
#kio_mobile_la_SOURCES = kio_mobile.cpp 
#kio_mobile_la_LIBADD = $(LIB_KIO) ../libkmobileclient.la
#kio_mobile_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module $(KDE_PLUGIN)
#
#kde_services_DATA = mobile.protocol cellphone.protocol \
#	organizer.protocol pda.protocol 
## do not install the camera protocol now - we already have a 
## camera kioslave !!
##       phonecamera.protocol
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/kio_mobile.pot
