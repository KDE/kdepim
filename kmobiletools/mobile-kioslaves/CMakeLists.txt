
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(P2KIOSLAVE)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(OBEXIOSLAVE)")
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(kio_mobile_PART_SRCS mobile.cpp mobileimpl.cpp )

kde4_automoc(${kio_mobile_PART_SRCS})

kde4_add_plugin(kio_mobile ${kio_mobile_PART_SRCS})

target_link_libraries(kio_mobile  ${KDE4_KDECORE_LIBS} kio kmobiletools )

install(TARGETS kio_mobile DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  mobile.protocol DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES  mobile.desktop DESTINATION ${DATA_INSTALL_DIR}/systemview )



#original Makefile.am contents follow:

#
#INCLUDES = -I$(srcdir)/../kmobiletools/libkmobiletools \
#	-I../kmobiletools/libkmobiletools \
#	$(all_includes)
#
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = kio_mobile.la
#
#kio_mobile_la_SOURCES = mobile.cpp mobileimpl.cpp
#kio_mobile_la_LIBADD = -lkio ../kmobiletools/libkmobiletools/libkmobiletools.la
#kio_mobile_la_LDFLAGS = -module -no-undefined $(all_libraries) $(KDE_PLUGIN)
#
#protocol_DATA = mobile.protocol
#protocoldir = $(kde_servicesdir)
#
#systemview_DATA = mobile.desktop
#systemviewdir = $(kde_datadir)/systemview
#
##messages: rc.cpp
##	$(EXTRACTRC) `find . -name \*.ui -o -name \*.rc` > rc.cpp
##	$(XGETTEXT) *.cpp -o $(podir)/kio_mobile.pot
#
#noinst_HEADERS = mobileimpl.h
#if with_p2kmoto
# P2KIOSLAVE = p2kslave
#endif
#
#if compile_obex
# OBEXIOSLAVE = obex
#endif
#
#SUBDIRS = $(P2KIOSLAVE) $(OBEXIOSLAVE)
#KDE_CXXFLAGS = -UQT_NO_ASCII_CAST
