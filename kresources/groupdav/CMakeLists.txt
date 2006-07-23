
include_directories(
	${CMAKE_SOURCE_DIR}/kresources/lib
	${CMAKE_BINARY_DIR}/kresources/lib
	
)


set(libkgroupdav_SRCS groupdavglobals.cpp)

set(kde_servicesdir     ${CMAKE_INSTALL_PREFIX}${SERVICES_INSTALL_DIR})

########### next target ###############

set(kcal_groupdav_LIB_SRCS groupdavcalendaradaptor.cpp kcal_resourcegroupdav.cpp ${libkgroupdav_SRCS})

kde4_automoc(${kcal_groupdav_LIB_SRCS})

kde4_add_library(kcal_groupdav SHARED ${kcal_groupdav_LIB_SRCS})

target_link_libraries(kcal_groupdav  ${KDE4_KDECORE_LIBS} kgroupwaredav ${KDE4_KCAL_LIBS} kdepim kcal_groupdav )

set_target_properties(kcal_groupdav PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kcal_groupdav  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(kabc_groupdav_LIB_SRCS kabc_resourcegroupdav.cpp groupdavaddressbookadaptor.cpp ${libkgroupdav_SRCS})

kde4_automoc(${kabc_groupdav_LIB_SRCS})

kde4_add_library(kabc_groupdav SHARED ${kabc_groupdav_LIB_SRCS})

target_link_libraries(kabc_groupdav  ${KDE4_KDECORE_LIBS} kgroupwaredav ${KDE4_KCAL_LIBS} kdepim ${KDE4_KABC_LIBS} kabc_groupdav )

set_target_properties(kabc_groupdav PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kabc_groupdav  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############


########### next target ###############

set(kcal_groupdav_PART_SRCS kcal_resourcegroupdav_plugin.cpp)

kde4_automoc(${kcal_groupdav_PART_SRCS})

kde4_add_plugin(kcal_groupdav ${kcal_groupdav_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcal_groupdav )

target_link_libraries(kcal_groupdav  ${KDE4_KDECORE_LIBS} )

install(TARGETS kcal_groupdav  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kabc_groupdav_PART_SRCS kabc_resourcegroupdav_plugin.cpp )

kde4_automoc(${kabc_groupdav_PART_SRCS})

kde4_add_plugin(kabc_groupdav ${kabc_groupdav_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kabc_groupdav )

target_link_libraries(kabc_groupdav  ${KDE4_KDECORE_LIBS} )

install(TARGETS kabc_groupdav  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kabc_groupdav.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kabc)
install( FILES kcal_groupdav.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kcal)

install(FILES uninstall.desktop DESTINATION ${kde_servicesdir}/kresources/kabc/ RENAME kabc_opengroupware.desktop)
install(FILES uninstall.desktop DESTINATION ${kde_servicesdir}/kresources/kcal/ RENAME kcal_opengroupware.desktop)











#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kresources/lib \
#           -I../lib $(all_includes)
#
#lib_LTLIBRARIES = libkcal_groupdav.la libkabc_groupdav.la
#noinst_LTLIBRARIES = libkgroupdav.la
#
#libkgroupdav_la_SOURCES = groupdavglobals.cpp
#
#libkcal_groupdav_la_SOURCES = groupdavcalendaradaptor.cpp kcal_resourcegroupdav.cpp 
#libkcal_groupdav_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
#                               -version-info 1:0:0 -no-undefined
#libkcal_groupdav_la_LIBADD = \
#  libkgroupdav.la ../lib/libkgroupwaredav.la \
#  $(top_builddir)/libkcal/libkcal.la $(top_builddir)/libkdepim/libkdepim.la 
#
#libkabc_groupdav_la_SOURCES = kabc_resourcegroupdav.cpp groupdavaddressbookadaptor.cpp 
#libkabc_groupdav_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
#                               -version-info 1:0:0 -no-undefined
#libkabc_groupdav_la_LIBADD = \
#  libkgroupdav.la ../lib/libkgroupwaredav.la  \
#  $(top_builddir)/libkcal/libkcal.la  $(top_builddir)/libkdepim/libkdepim.la \
#  -lkabc
#
#kde_module_LTLIBRARIES = kcal_groupdav.la kabc_groupdav.la
#
#kcal_groupdav_la_SOURCES = kcal_resourcegroupdav_plugin.cpp
#kcal_groupdav_la_LDFLAGS = $(all_libraries) -module -no-undefined $(KDE_PLUGIN)
#kcal_groupdav_la_LIBADD = libkcal_groupdav.la
#
#kabc_groupdav_la_SOURCES = kabc_resourcegroupdav_plugin.cpp
#kabc_groupdav_la_LDFLAGS = $(all_libraries) -module -no-undefined $(KDE_PLUGIN)
#kabc_groupdav_la_LIBADD = libkabc_groupdav.la
#
#kcal_servicedir = $(kde_servicesdir)/kresources/${KDE4_KCAL_LIBS}
#kcal_service_DATA = kcal_groupdav.desktop
#
#kabc_servicedir = $(kde_servicesdir)/kresources/kabc
#kabc_service_DATA = kabc_groupdav.desktop
#
#METASOURCES = AUTO
#
#install-data-local: uninstall.desktop
#	$(mkinstalldirs) $(DESTDIR)$(kde_servicesdir)/kresources/kabc
#	$(mkinstalldirs) $(DESTDIR)$(kde_servicesdir)/kresources/${KDE4_KCAL_LIBS}
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_servicesdir)/kresources/kabc/kabc_opengroupware.desktop
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_servicesdir)/kresources/${KDE4_KCAL_LIBS}/kcal_opengroupware.desktop
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kres_groupdav.pot
