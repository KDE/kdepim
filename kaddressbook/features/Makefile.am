INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kaddressbook $(all_includes) 

kde_module_LTLIBRARIES = libkaddrbk_distributionlist.la \
 libkaddrbk_location.la

noinst_HEADERS = distributionlistwidget.h locationwidget.h

servicedir  = $(kde_servicesdir)/kaddressbook
service_DATA = distributionlist.desktop location.desktop

libkaddrbk_distributionlist_la_SOURCES = distributionlistwidget.cpp
libkaddrbk_distributionlist_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries)
libkaddrbk_distributionlist_la_LIBADD = $(top_builddir)/kaddressbook/libkaddressbookpart.la
libkaddrbk_distributionlist_la_METASOURCES = AUTO

libkaddrbk_location_la_SOURCES = locationwidget.cpp
libkaddrbk_location_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries)
libkaddrbk_location_la_LIBADD = $(top_builddir)/kaddressbook/libkaddressbookpart.la
libkaddrbk_location_la_METASOURCES = AUTO
