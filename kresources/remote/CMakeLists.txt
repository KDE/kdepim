



########### next target ###############

set(kcal_resourceremote_LIB_SRCS resourceremote.cpp resourceremoteconfig.cpp )

kde4_automoc(${kcal_resourceremote_LIB_SRCS})

kde4_add_library(kcal_resourceremote SHARED ${kcal_resourceremote_LIB_SRCS})

target_link_libraries(kcal_resourceremote  ${KDE4_KDECORE_LIBS} ${KDE4_KCAL_LIBS} kdepim )

set_target_properties(kcal_resourceremote PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kcal_resourceremote  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(kcal_remote_PART_SRCS resourceremote_plugin.cpp )

kde4_automoc(${kcal_remote_PART_SRCS})

kde4_add_plugin(kcal_remote ${kcal_remote_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcal_remote )

target_link_libraries(kcal_remote  ${KDE4_KDECORE_LIBS} kcal_resourceremote kdepim )

install(TARGETS kcal_remote  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES remote.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kcal)
install( FILES resourceremoteconfig.h resourceremote.h  DESTINATION ${INCLUDE_INSTALL_DIR}/kcal)




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) $(all_includes)
#
#lib_LTLIBRARIES   = libkcal_resourceremote.la
#libkcal_resourceremote_la_LDFLAGS = $(all_libraries) -no-undefined \
#                                    -version-info 1:0:0
#libkcal_resourceremote_la_LIBADD  = $(top_builddir)/libkcal/libkcal.la \
#                                    $(top_builddir)/libkdepim/libkdepim.la
#libkcal_resourceremote_la_SOURCES = resourceremote.cpp resourceremoteconfig.cpp
#
#kde_module_LTLIBRARIES = kcal_remote.la
#
#kcal_remote_la_SOURCES = resourceremote_plugin.cpp
#kcal_remote_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kcal_remote_la_LIBADD  = libkcal_resourceremote.la \
#                         $(top_builddir)/libkdepim/libkdepim.la
#
#libkcalincludedir = $(includedir)/libkcal
#libkcalinclude_HEADERS = resourceremoteconfig.h resourceremote.h
#
#servicedir = $(kde_servicesdir)/kresources/${KDE4_KCAL_LIBS}
#service_DATA = remote.desktop
#
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kres_remote.pot
