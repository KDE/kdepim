kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kresources/kolab/shared ${CMAKE_SOURCE_DIR}/knotes ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(knoteskolab_LIB_SRCS resourcekolab.cpp note.cpp )

kde4_automoc(${knoteskolab_LIB_SRCS})

kde4_add_library(knoteskolab SHARED ${knoteskolab_LIB_SRCS})

target_link_libraries(knoteskolab  ${QT_AND_KDECORE_LIBS} kresources kdeprint resourcekolabshared knotesresources kcal )

set_target_properties(knoteskolab PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} knoteskolab )


########### next target ###############

set(knotes_kolab_PART_SRCS resourcekolab_plugin.cpp )

kde4_automoc(${knotes_kolab_PART_SRCS})

kde4_add_plugin(knotes_kolab ${knotes_kolab_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} knotes_kolab )

target_link_libraries(knotes_kolab  ${QT_AND_KDECORE_LIBS} knoteskolab )

install_targets(${LIB_INSTALL_DIR}/kde4 knotes_kolab )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kresources/knotes FILES kolabresource.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#INCLUDES = -I$(top_srcdir)/kresources/kolab/shared \
#	-I$(top_srcdir) -I$(top_srcdir)/knotes -I$(top_builddir)/libkdepim $(all_includes)
#
## The kolab wizard links to this library too
#lib_LTLIBRARIES = libknoteskolab.la
#
#libknoteskolab_la_SOURCES = resourcekolab.cpp note.cpp
#libknoteskolab_la_LDFLAGS = $(all_libraries) -no-undefined
#libknoteskolab_la_LIBADD = -lkresources -lkdeprint \
#	$(top_builddir)/kresources/kolab/shared/libresourcekolabshared.la \
#	$(top_builddir)/knotes/libknotesresources.la \
#	$(top_builddir)/libkcal/libkcal.la
#
#kde_module_LTLIBRARIES = knotes_kolab.la
#
#knotes_kolab_la_SOURCES = resourcekolab_plugin.cpp
#knotes_kolab_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) -no-undefined
#knotes_kolab_la_LIBADD = libknoteskolab.la
#
#servicedir = $(kde_servicesdir)/kresources/knotes
#service_DATA = kolabresource.desktop
#
#install-data-local: $(srcdir)/../uninstall.desktop
#	$(mkinstalldirs) $(DESTDIR)$(servicedir)
#	$(INSTALL_DATA) $(srcdir)/../uninstall.desktop $(DESTDIR)$(servicedir)/imap.desktop
#
