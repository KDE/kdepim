INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kresources/lib \
           -I../lib $(all_includes)

lib_LTLIBRARIES = libkcal_groupdav.la libkabc_groupdav.la
noinst_LTLIBRARIES = libkgroupdav.la

libkgroupdav_la_SOURCES = groupdavglobals.cpp

libkcal_groupdav_la_SOURCES = groupdavcalendaradaptor.cpp kcal_resourcegroupdav.cpp 
libkcal_groupdav_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
                               -version-info 1:0:0 -no-undefined
libkcal_groupdav_la_LIBADD = \
  libkgroupdav.la ../lib/libkgroupwaredav.la \
  $(top_builddir)/libkcal/libkcal.la $(top_builddir)/libkdepim/libkdepim.la 

libkabc_groupdav_la_SOURCES = kabc_resourcegroupdav.cpp groupdavaddressbookadaptor.cpp 
libkabc_groupdav_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
                               -version-info 1:0:0 -no-undefined
libkabc_groupdav_la_LIBADD = \
  libkgroupdav.la ../lib/libkgroupwaredav.la  \
  $(top_builddir)/libkcal/libkcal.la  $(top_builddir)/libkdepim/libkdepim.la \
  -lkabc

kde_module_LTLIBRARIES = kcal_groupdav.la kabc_groupdav.la

kcal_groupdav_la_SOURCES = kcal_resourcegroupdav_plugin.cpp
kcal_groupdav_la_LDFLAGS = $(all_libraries) -module -no-undefined $(KDE_PLUGIN)
kcal_groupdav_la_LIBADD = libkcal_groupdav.la

kabc_groupdav_la_SOURCES = kabc_resourcegroupdav_plugin.cpp
kabc_groupdav_la_LDFLAGS = $(all_libraries) -module -no-undefined $(KDE_PLUGIN)
kabc_groupdav_la_LIBADD = libkabc_groupdav.la

kcal_servicedir = $(kde_servicesdir)/kresources/kcal
kcal_service_DATA = kcal_groupdav.desktop

kabc_servicedir = $(kde_servicesdir)/kresources/kabc
kabc_service_DATA = kabc_groupdav.desktop

METASOURCES = AUTO

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(kde_servicesdir)/kresources/kabc
	$(mkinstalldirs) $(DESTDIR)$(kde_servicesdir)/kresources/kcal
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_servicesdir)/kresources/kabc/kabc_opengroupware.desktop
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_servicesdir)/kresources/kcal/kcal_opengroupware.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kres_groupdav.pot
