INCLUDES = -I$(top_srcdir) -I$(top_builddir)/libkdepim -I$(top_srcdir)/libkdepim \
           -I$(top_srcdir)/kresources/lib \
					 -I$(top_builddir)/kresources/lib $(all_includes)

lib_LTLIBRARIES = libkabc_newexchange.la libkcal_newexchange.la 
noinst_LTLIBRARIES = libknewexchange.la

libknewexchange_la_SOURCES = exchangeglobals.cpp exchangeconvertercalendar.cpp exchangeconvertercontact.cpp

libkcal_newexchange_la_SOURCES = exchangecalendaradaptor.cpp kcal_resourceexchange.cpp 
libkcal_newexchange_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
                               -version-info 1:0:0 -no-undefined
libkcal_newexchange_la_LIBADD = \
  libknewexchange.la ../lib/libkgroupwaredav.la \
  $(top_builddir)/libkcal/libkcal.la $(top_builddir)/libkdepim/libkdepim.la \
  -lkabc
	
libkabc_newexchange_la_SOURCES = kabc_resourceexchange.cpp exchangeaddressbookadaptor.cpp
libkabc_newexchange_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
                               -version-info 1:0:0 -no-undefined
libkabc_newexchange_la_LIBADD = \
  libknewexchange.la ../lib/libkgroupwaredav.la \
  $(top_builddir)/libkcal/libkcal.la $(top_builddir)/libkdepim/libkdepim.la \
  -lkabc

kde_module_LTLIBRARIES = kcal_newexchange.la kabc_newexchange.la

kcal_newexchange_la_SOURCES = kcal_resourceexchange_plugin.cpp
kcal_newexchange_la_LDFLAGS = $(all_libraries) -module -no-undefined $(KDE_PLUGIN)
kcal_newexchange_la_LIBADD = libkcal_newexchange.la

kabc_newexchange_la_SOURCES = kabc_resourceexchange_plugin.cpp
kabc_newexchange_la_LDFLAGS = $(all_libraries) -module -no-undefined $(KDE_PLUGIN)
kabc_newexchange_la_LIBADD = libkabc_newexchange.la


kcal_servicedir = $(kde_servicesdir)/kresources/kcal
kcal_service_DATA = kcal_newexchange.desktop

kabc_servicedir = $(kde_servicesdir)/kresources/kabc
kabc_service_DATA = kabc_newexchange.desktop

METASOURCES = AUTO
