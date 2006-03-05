kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kresources/groupwise/soap ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kio_groupwise_PART_SRCS groupwise.cpp )

kde4_automoc(${kio_groupwise_PART_SRCS})

kde4_add_plugin(kio_groupwise ${kio_groupwise_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kio_groupwise )

target_link_libraries(kio_groupwise  ${QT_AND_KDECORE_LIBS} kcal gwsoap kdepim kio )

install_targets(${LIB_INSTALL_DIR}/kde4 kio_groupwise )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES groupwise.protocol groupwises.protocol )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kresources/groupwise/soap \
#  -I$(top_builddir)/libkdepim $(all_includes)
#
#noinst_HEADERS = groupwise.h
#
#METASOURCES = AUTO
#
#kdelnkdir = $(kde_servicesdir)
#kdelnk_DATA = groupwise.protocol groupwises.protocol
#
#kde_module_LTLIBRARIES =  kio_groupwise.la
#
#kio_groupwise_la_SOURCES = groupwise.cpp
#kio_groupwise_la_LIBADD = $(top_builddir)/libkcal/libkcal.la \
#  ../soap/libgwsoap.la $(top_builddir)/libkdepim/libkdepim.la $(LIB_KIO)
#kio_groupwise_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kio_groupwise.pot
