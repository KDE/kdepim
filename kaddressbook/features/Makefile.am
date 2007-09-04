INCLUDES = -I$(top_srcdir)/kaddressbook/interfaces -I$(top_srcdir)/kaddressbook \
           -I$(top_srcdir)  $(all_includes)

if compile_newdistrlists
TARGET_DISTRIBUTIONLISTNG=libkaddrbk_distributionlistng
DISTRIBUTIONLISTNG_DESKTOPFILE=distributionlistng.desktop
endif

kde_module_LTLIBRARIES = $(TARGET_DISTRIBUTIONLISTNG) libkaddrbk_distributionlist.la libkaddrbk_resourceselection.la

XXLIBS = $(top_builddir)/kaddressbook/interfaces/libkabinterfaces.la \
         $(top_builddir)/libkdepim/libkdepim.la

libkaddrbk_distributionlist_la_SOURCES = distributionlistwidget.cpp
libkaddrbk_distributionlist_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
libkaddrbk_distributionlist_la_LIBADD = $(XXLIBS)

if compile_newdistrlists
libkaddrbk_distributionlistng_la_SOURCES = distributionlistngwidget.cpp
libkaddrbk_distributionlistng_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
libkaddrbk_distributionlistng_la_LIBADD = $(XXLIBS)
endif

libkaddrbk_resourceselection_la_SOURCES = resourceselection.cpp
libkaddrbk_resourceselection_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
libkaddrbk_resourceselection_la_LIBADD = $(XXLIBS)

noinst_HEADERS = distributionlistwidget.h distributionlistngwidget.h resourceselection.h

METASOURCES = AUTO

servicedir  = $(kde_servicesdir)/kaddressbook
service_DATA = $(DISTRIBUTIONLISTNG_DESKTOPFILE) distributionlist.desktop resourceselection.desktop
