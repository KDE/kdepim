kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kio_sieve_PART_SRCS sieve.cpp )

kde4_automoc(${kio_sieve_PART_SRCS})

kde4_add_plugin(kio_sieve ${kio_sieve_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kio_sieve )

target_link_libraries(kio_sieve  ${QT_AND_KDECORE_LIBS} kio )

install_targets(${LIB_INSTALL_DIR}/kde4 kio_sieve )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES sieve.protocol )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= -I$(srcdir)/../.. -I$(srcdir)/.. $(all_includes)
#
#kde_module_LTLIBRARIES = kio_sieve.la
#
#kio_sieve_la_SOURCES = sieve.cpp
#kio_sieve_la_LIBADD  = $(LIB_KIO) $(SASL2_LIBS)
#kio_sieve_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#
#noinst_HEADERS = sieve.h
#
#kdelnk_DATA = sieve.protocol
#kdelnkdir = $(kde_servicesdir)
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/kio_sieve.pot
