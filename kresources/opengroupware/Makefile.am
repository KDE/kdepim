INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libkdepim $(all_includes)

lib_LTLIBRARIES = libkopengroupware.la libkcal_opengroupware.la \
  libkabc_opengroupware.la

libkopengroupware_la_SOURCES = folderlister.cpp folderconfig.cpp \
  webdavhandler.cpp
libkopengroupware_la_LDFLAGS = $(all_libraries)
libkopengroupware_la_LIBADD  = $(top_builddir)/libkcal/libkcal.la \
                      $(top_builddir)/libkdepim/libkdepim.la


libkcal_opengroupware_la_SOURCES = kcal_resourceopengroupware.cpp \
                               kcal_resourceopengroupwareconfig.cpp \
                               kcal_opengroupwareprefsbase.kcfgc
libkcal_opengroupware_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
                               -version-info 1:0:0 -no-undefined
libkcal_opengroupware_la_LIBADD = \
  libkopengroupware.la \
  $(top_builddir)/libkcal/libkcal.la $(top_builddir)/libkdepim/libkdepim.la 

libkabc_opengroupware_la_SOURCES = kabc_resourceopengroupware.cpp \
                               kabc_resourceopengroupwareconfig.cpp \
                               kabc_opengroupwareprefs.kcfgc
libkabc_opengroupware_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
                               -version-info 1:0:0 -no-undefined
libkabc_opengroupware_la_LIBADD = libkopengroupware.la \
  $(top_builddir)/libkcal/libkcal.la  $(top_builddir)/libkdepim/libkdepim.la \
  -lkabc

kde_module_LTLIBRARIES = kcal_opengroupware.la kabc_opengroupware.la

kcal_opengroupware_la_SOURCES = kcal_resourceopengroupware_plugin.cpp
kcal_opengroupware_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kcal_opengroupware_la_LIBADD = libkcal_opengroupware.la

kabc_opengroupware_la_SOURCES = kabc_resourceopengroupware_plugin.cpp
kabc_opengroupware_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kabc_opengroupware_la_LIBADD = libkabc_opengroupware.la

kcal_resourceopengroupware_plugin.cpp: kcal_opengroupwareprefsbase.h
kabc_resourceopengroupware_plugin.cpp: kcal_opengroupwareprefsbase.h

kcal_servicedir = $(kde_servicesdir)/kresources/kcal
kcal_service_DATA = kcal_opengroupware.desktop

kabc_servicedir = $(kde_servicesdir)/kresources/kabc
kabc_service_DATA = kabc_opengroupware.desktop

METASOURCES = AUTO
