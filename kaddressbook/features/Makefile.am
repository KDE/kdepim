INCLUDES = -I$(top_srcdir)/kaddressbook/interfaces -I$(top_srcdir)/kaddressbook \
           -I$(top_srcdir)  $(all_includes) 

kde_module_LTLIBRARIES = libkaddrbk_distributionlist.la libkaddrbk_location.la

XXLIBS = $(top_builddir)/kaddressbook/interfaces/libkabinterfaces.la \
         $(top_builddir)/libkdepim/libkdepim.la

libkaddrbk_distributionlist_la_SOURCES = distributionlistwidget.cpp
libkaddrbk_distributionlist_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
libkaddrbk_distributionlist_la_LIBADD = $(XXLIBS)

libkaddrbk_location_la_SOURCES = locationwidget.cpp locationconfig.cpp
libkaddrbk_location_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
libkaddrbk_location_la_LIBADD = $(XXLIBS)

noinst_HEADERS = distributionlistwidget.h locationwidget.h locationconfig.h

METASOURCES = AUTO

servicedir  = $(kde_servicesdir)/kaddressbook
service_DATA = distributionlist.desktop location.desktop
