kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kresources/kolab/shared ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcalkolab_LIB_SRCS 
   incidence.cpp 
   event.cpp 
   task.cpp 
   journal.cpp 
   resourcekolab.cpp )

kde4_automoc(${kcalkolab_LIB_SRCS})

kde4_add_library(kcalkolab SHARED ${kcalkolab_LIB_SRCS})

target_link_libraries(kcalkolab  ${QT_AND_KDECORE_LIBS} kcal resourcekolabshared kresources )

set_target_properties(kcalkolab PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kcalkolab )


########### next target ###############

set(kcal_kolab_PART_SRCS resourcekolab_plugin.cpp )

kde4_automoc(${kcal_kolab_PART_SRCS})

kde4_add_plugin(kcal_kolab ${kcal_kolab_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcal_kolab )

target_link_libraries(kcal_kolab  ${QT_AND_KDECORE_LIBS} kcalkolab )

install_targets(${LIB_INSTALL_DIR}/kde4 kcal_kolab )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kresources/kcal FILES kolab.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#INCLUDES = -I$(top_srcdir)/kresources/kolab/shared -I$(top_srcdir) \
#        -I$(top_builddir)/libkdepim $(all_includes)
#
## The kolab wizard links to this library too
#lib_LTLIBRARIES = libkcalkolab.la
#
#libkcalkolab_la_SOURCES = incidence.cpp event.cpp task.cpp journal.cpp resourcekolab.cpp
#libkcalkolab_la_LDFLAGS = $(all_libraries) -no-undefined
#libkcalkolab_la_LIBADD = $(top_builddir)/libkcal/libkcal.la \
#	$(top_builddir)/kresources/kolab/shared/libresourcekolabshared.la \
#	-lkresources
#
#kde_module_LTLIBRARIES = kcal_kolab.la
#
#kcal_kolab_la_SOURCES = resourcekolab_plugin.cpp
#kcal_kolab_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) -no-undefined
#kcal_kolab_la_LIBADD = libkcalkolab.la
#
#servicedir = $(kde_servicesdir)/kresources/kcal
#service_DATA = kolab.desktop
#
#install-data-local: $(srcdir)/../uninstall.desktop
#	$(mkinstalldirs) $(DESTDIR)$(servicedir)
#	$(INSTALL_DATA) $(srcdir)/../uninstall.desktop $(DESTDIR)$(servicedir)/imap.desktop
#
