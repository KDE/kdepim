INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libkdepim $(all_includes)

lib_LTLIBRARIES = libkcal_groupware.la

libkcal_groupware_la_SOURCES = kcal_resourcegroupware.cpp \
                               kcal_resourcegroupwareconfig.cpp \
                               kcal_groupwareprefsbase.kcfgc \
                               confirmsavedialog.cpp
libkcal_groupware_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
                               -version-info 1:0:0 -no-undefined
libkcal_groupware_la_LIBADD = \
                              $(top_builddir)/libkcal/libkcal.la \
                              $(top_builddir)/libkdepim/libkdepim.la

kde_module_LTLIBRARIES = kcal_groupware.la

kcal_groupware_la_SOURCES = kcal_resourcegroupware_plugin.cpp
kcal_groupware_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kcal_groupware_la_LIBADD = libkcal_groupware.la

kcal_servicedir = $(kde_servicesdir)/kresources/kcal
kcal_service_DATA = kcal_groupware.desktop

METASOURCES = AUTO
