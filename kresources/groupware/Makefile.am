INCLUDES = -I$(top_srcdir) -I$(top_builddir)/libkdepim $(all_includes)

lib_LTLIBRARIES = libkcal_groupware.la libkabc_groupware.la

libkcal_groupware_la_SOURCES = kcal_resourcegroupware.cpp \
                               kcal_resourcegroupwareconfig.cpp \
                               kcal_groupwareprefsbase.kcfgc
libkcal_groupware_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
                               -version-info 1:0:0 -no-undefined
libkcal_groupware_la_LIBADD = \
                              $(top_builddir)/libkcal/libkcal.la \
                              $(top_builddir)/libkdepim/libkdepim.la

libkabc_groupware_la_SOURCES = kabc_resourcegroupware.cpp \
                               kabc_resourcegroupwareconfig.cpp \
                               kabc_groupwareprefs.kcfgc
libkabc_groupware_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
                               -version-info 1:0:0 -no-undefined
libkabc_groupware_la_LIBADD = $(top_builddir)/libkcal/libkcal.la \
                              $(top_builddir)/libkdepim/libkdepim.la \
                              -lkabc

kde_module_LTLIBRARIES = kcal_groupware.la kabc_groupware.la

kcal_groupware_la_SOURCES = kcal_resourcegroupware_plugin.cpp
kcal_groupware_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kcal_groupware_la_LIBADD = libkcal_groupware.la

kabc_groupware_la_SOURCES = kabc_resourcegroupware_plugin.cpp
kabc_groupware_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kabc_groupware_la_LIBADD = libkabc_groupware.la

kcal_servicedir = $(kde_servicesdir)/kresources/kcal
kcal_service_DATA = kcal_groupware.desktop

kabc_servicedir = $(kde_servicesdir)/kresources/kabc
kabc_service_DATA = kabc_groupware.desktop

METASOURCES = AUTO
