INCLUDES = -I$(top_srcdir)/kaddressbook/interfaces -I$(top_srcdir)/kaddressbook \
           -I$(top_srcdir)  $(all_includes)

kde_module_LTLIBRARIES = libkaddrbk_distributionlist.la libkaddrbk_distributionlistng.la libkaddrbk_resourceselection.la

XXLIBS = $(top_builddir)/kaddressbook/interfaces/libkabinterfaces.la \
         $(top_builddir)/libkdepim/libkdepim.la

libkaddrbk_distributionlist_la_SOURCES = distributionlistwidget.cpp
libkaddrbk_distributionlist_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
libkaddrbk_distributionlist_la_LIBADD = $(XXLIBS)

libkaddrbk_distributionlistng_la_SOURCES = distributionlistngwidget.cpp
libkaddrbk_distributionlistng_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
libkaddrbk_distributionlistng_la_LIBADD = $(XXLIBS)

libkaddrbk_resourceselection_la_SOURCES = resourceselection.cpp
libkaddrbk_resourceselection_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
libkaddrbk_resourceselection_la_LIBADD = $(XXLIBS)

noinst_HEADERS = distributionlistwidget.h distributionlistngwidget.h resourceselection.h

METASOURCES = AUTO

servicedir  = $(kde_servicesdir)/kaddressbook
service_DATA = distributionlistng.desktop distributionlist.desktop resourceselection.desktop
