SUBDIRS = soap kioslave

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libkdepim $(all_includes)

lib_LTLIBRARIES = libkcal_groupwise.la libkabc_groupwise.la

libkcal_groupwise_la_SOURCES = kcal_resourcegroupwise.cpp \
                               kcal_resourcegroupwiseconfig.cpp \
                               kcal_groupwiseprefs.kcfgc \
                               confirmsavedialog.cpp
libkcal_groupwise_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
                               -version-info 1:0:0 -no-undefined
libkcal_groupwise_la_LIBADD = soap/libgwsoap.la \
                              $(top_builddir)/libkcal/libkcal.la \
                              $(top_builddir)/libkdepim/libkdepim.la

libkabc_groupwise_la_SOURCES = kabc_resourcegroupwise.cpp \
                               kabc_resourcegroupwiseconfig.cpp \
                               kabc_groupwiseprefs.kcfgc
libkabc_groupwise_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
                               -version-info 1:0:0 -no-undefined
libkabc_groupwise_la_LIBADD = soap/libgwsoap.la \
                              $(top_builddir)/libkcal/libkcal.la \
                              $(top_builddir)/libkdepim/libkdepim.la \
                              -lkabc


kde_module_LTLIBRARIES = kcal_groupwise.la kabc_groupwise.la

kcal_groupwise_la_SOURCES = kcal_resourcegroupwise_plugin.cpp
kcal_groupwise_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kcal_groupwise_la_LIBADD = libkcal_groupwise.la

kabc_groupwise_la_SOURCES = kabc_resourcegroupwise_plugin.cpp
kabc_groupwise_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kabc_groupwise_la_LIBADD = libkabc_groupwise.la

kcal_servicedir = $(kde_servicesdir)/kresources/kcal
kcal_service_DATA = kcal_groupwise.desktop

kabc_servicedir = $(kde_servicesdir)/kresources/kabc
kabc_service_DATA = kabc_groupwise.desktop

METASOURCES = AUTO
