
include_directories( ${CMAKE_SOURCE_DIR}/kmobiletools/kmobiletools/libkmobiletools ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(kio_obex2_PART_SRCS kio_obexprotocol.cpp obeximpl.cpp obexwrapper.cpp )


kde4_add_plugin(kio_obex2 ${kio_obex2_PART_SRCS})

target_link_libraries(kio_obex2  ${KDE4_KIO_LIBS} bfb multicobex bluetooth openobex obexftp )

install(TARGETS kio_obex2 DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  obex2.protocol DESTINATION ${SERVICES_INSTALL_DIR} )



#original Makefile.am contents follow:

#KDE_OPTIONS = nofinal
#
#
#INCLUDES = -I$(top_srcdir)/kmobiletools/kmobiletools/libkmobiletools \
#           -I$(top_builddir)/kmobiletools/kmobiletools/libkmobiletools \
#	   $(all_includes)
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = kio_obex2.la
#
#kio_obex2_la_SOURCES = kio_obexprotocol.cpp obeximpl.cpp obexwrapper.cpp 
###deviceIFace.stub
#kio_obex2_la_LIBADD = -lkio -lbfb -lmulticobex -lbluetooth -lopenobex -lobexftp 
#kio_obex2_la_LDFLAGS = -module -no-undefined -avoid-version $(all_libraries) $(KDE_PLUGIN)
#
#protocol_DATA = obex2.protocol
#protocoldir = $(kde_servicesdir)
#
###deviceIFace_DIR=$(top_srcdir)/kmobiletools/kmobiletools/libkmobiletools
#noinst_HEADERS = kio_obexprotocol.h obeximpl.h obexwrapper.h
#
#messages: rc.cpp
#	$(EXTRACTRC) `find . -name \*.ui -o -name \*.rc` > rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kio_obex.pot
#KDE_CXXFLAGS = -UQT_NO_ASCII_CAST
#
