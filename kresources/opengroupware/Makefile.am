INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libkdepim $(all_includes)

lib_LTLIBRARIES = libkcal_opengroupware.la

libkcal_opengroupware_la_SOURCES = kcal_resourceopengroupware.cpp \
                               kcal_resourceopengroupwareconfig.cpp \
                               kcal_opengroupwareprefsbase.kcfgc \
                               webdavhandler.cpp \
                               folderconfig.cpp folderlister.cpp
libkcal_opengroupware_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
                               -version-info 1:0:0 -no-undefined
libkcal_opengroupware_la_LIBADD = \
  $(top_builddir)/libkcal/libkcal.la $(top_builddir)/libkdepim/libkdepim.la 

kde_module_LTLIBRARIES = kcal_opengroupware.la

kcal_opengroupware_la_SOURCES = kcal_resourceopengroupware_plugin.cpp
kcal_opengroupware_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kcal_opengroupware_la_LIBADD = libkcal_opengroupware.la

kcal_servicedir = $(kde_servicesdir)/kresources/kcal
kcal_service_DATA = kcal_opengroupware.desktop

METASOURCES = AUTO
