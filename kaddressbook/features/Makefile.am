INCLUDES = -I$(top_srcdir)/kaddressbook/interfaces -I$(top_srcdir)/kaddressbook \
           -I$(top_srcdir)  $(all_includes) 

kde_module_LTLIBRARIES = libkaddrbk_distributionlist.la

XXLIBS = $(top_builddir)/kaddressbook/interfaces/libkabinterfaces.la \
         $(top_builddir)/libkdepim/libkdepim.la

libkaddrbk_distributionlist_la_SOURCES = distributionlistwidget.cpp
libkaddrbk_distributionlist_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
libkaddrbk_distributionlist_la_LIBADD = $(XXLIBS)

noinst_HEADERS = distributionlistwidget.h

METASOURCES = AUTO

servicedir  = $(kde_servicesdir)/kaddressbook
service_DATA = distributionlist.desktop
