kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kresources/kolab/shared ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kabckolab_LIB_SRCS resourcekolab.cpp contact.cpp )

kde4_automoc(${kabckolab_LIB_SRCS})

kde4_add_library(kabckolab SHARED ${kabckolab_LIB_SRCS})

target_link_libraries(kabckolab  ${QT_AND_KDECORE_LIBS} resourcekolabshared kresources kabc )

set_target_properties(kabckolab PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kabckolab )


########### next target ###############

set(kabc_kolab_PART_SRCS resourcekolab_plugin.cpp )

kde4_automoc(${kabc_kolab_PART_SRCS})

kde4_add_plugin(kabc_kolab ${kabc_kolab_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kabc_kolab )

target_link_libraries(kabc_kolab  ${QT_AND_KDECORE_LIBS} kabckolab )

install_targets(${LIB_INSTALL_DIR}/kde4 kabc_kolab )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kresources/kabc FILES kolab.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#INCLUDES = -I$(top_srcdir)/kresources/kolab/shared -I$(top_srcdir) $(all_includes)
#
## The kolab wizard links to this library too
#lib_LTLIBRARIES = libkabckolab.la
#
#libkabckolab_la_SOURCES = resourcekolab.cpp contact.cpp
#libkabckolab_la_LDFLAGS = $(all_libraries) -no-undefined
#libkabckolab_la_LIBADD =  \
#	$(top_builddir)/kresources/kolab/shared/libresourcekolabshared.la \
#	-lkresources -lkabc
#
#kde_module_LTLIBRARIES = kabc_kolab.la
#
#noinst_HEADERS = resourcekolab.h contact.h
#
#kabc_kolab_la_SOURCES = resourcekolab_plugin.cpp
#kabc_kolab_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) -no-undefined
#kabc_kolab_la_LIBADD = libkabckolab.la
#
#servicedir = $(kde_servicesdir)/kresources/kabc
#service_DATA = kolab.desktop
#
#install-data-local: $(srcdir)/../uninstall.desktop
#	$(mkinstalldirs) $(DESTDIR)$(servicedir)
#	$(INSTALL_DATA) $(srcdir)/../uninstall.desktop $(DESTDIR)$(servicedir)/imap.desktop
