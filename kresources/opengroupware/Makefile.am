INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libkdepim $(all_includes)

lib_LTLIBRARIES = libkgroupwarebase.la libkcal_opengroupware.la \
  libkabc_opengroupware.la
noinst_LTLIBRARIES = libkopengroupware.la

libkgroupwarebase_la_SOURCES = folderlister.cpp folderconfig.cpp \
  webdavhandler.cpp groupwaredataadaptor.cpp groupwaredownloadjob.cpp \
  calendaradaptor.cpp addressbookadaptor.cpp groupwareresourcejob.cpp \
  groupwareuploadjob.cpp kcal_resourcegroupwarebase.cpp kabc_resourcegroupwarebase.cpp \
  kcal_groupwareprefs.kcfgc kabc_groupwareprefs.kcfgc \
	kcal_resourcegroupwarebaseconfig.cpp kabc_resourcegroupwarebaseconfig.cpp \
	davgroupwareglobals.cpp
libkgroupwarebase_la_LDFLAGS = $(all_libraries) -no-undefined
libkgroupwarebase_la_LIBADD  = $(top_builddir)/libkcal/libkcal.la \
                      $(top_builddir)/libkdepim/libkdepim.la

libkopengroupware_la_SOURCES = ogoglobals.cpp ogofolderlister.cpp

libkcal_opengroupware_la_SOURCES = ogocalendaradaptor.cpp kcal_resourceopengroupware.cpp 
libkcal_opengroupware_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
                               -version-info 1:0:0 -no-undefined
libkcal_opengroupware_la_LIBADD = \
  libkopengroupware.la libkgroupwarebase.la \
  $(top_builddir)/libkcal/libkcal.la $(top_builddir)/libkdepim/libkdepim.la 

libkabc_opengroupware_la_SOURCES = kabc_resourceopengroupware.cpp ogoaddressbookadaptor.cpp 
libkabc_opengroupware_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
                               -version-info 1:0:0 -no-undefined
libkabc_opengroupware_la_LIBADD = libkopengroupware.la libkgroupwarebase.la \
  $(top_builddir)/libkcal/libkcal.la  $(top_builddir)/libkdepim/libkdepim.la \
  -lkabc

kde_module_LTLIBRARIES = kcal_opengroupware.la kabc_opengroupware.la

kcal_opengroupware_la_SOURCES = kcal_resourceopengroupware_plugin.cpp
kcal_opengroupware_la_LDFLAGS = $(all_libraries) -module -no-undefined $(KDE_PLUGIN)
kcal_opengroupware_la_LIBADD = libkcal_opengroupware.la

kabc_opengroupware_la_SOURCES = kabc_resourceopengroupware_plugin.cpp
kabc_opengroupware_la_LDFLAGS = $(all_libraries) -module -no-undefined $(KDE_PLUGIN)
kabc_opengroupware_la_LIBADD = libkabc_opengroupware.la

kcal_servicedir = $(kde_servicesdir)/kresources/kcal
kcal_service_DATA = kcal_opengroupware.desktop

kabc_servicedir = $(kde_servicesdir)/kresources/kabc
kabc_service_DATA = kabc_opengroupware.desktop

METASOURCES = AUTO
