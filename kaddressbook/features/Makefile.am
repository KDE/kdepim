INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kaddressbook/interfaces -I$(top_srcdir)/kaddressbook $(all_includes) 

kde_module_LTLIBRARIES = libkaddrbk_distributionlist.la \
  libkaddrbk_location.la libkaddrbk_merge.la

noinst_HEADERS = distributionlistwidget.h locationwidget.h locationconfig.h \
  mergewidget.h

servicedir  = $(kde_servicesdir)/kaddressbook
service_DATA = distributionlist.desktop location.desktop merge.desktop

XXLIBS = $(top_builddir)/kaddressbook/interfaces/libkabinterfaces.la \
         $(top_builddir)/libkdepim/libkdepim.la

libkaddrbk_distributionlist_la_SOURCES = distributionlistwidget.cpp
libkaddrbk_distributionlist_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries)
libkaddrbk_distributionlist_la_LIBADD = $(XXLIBS)

libkaddrbk_location_la_SOURCES = locationwidget.cpp locationconfig.cpp
libkaddrbk_location_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries)
libkaddrbk_location_la_LIBADD = $(XXLIBS) $(LIB_KHTML)

libkaddrbk_merge_la_SOURCES = mergewidget.cpp
libkaddrbk_merge_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries)
libkaddrbk_merge_la_LIBADD = $(XXLIBS)

METASOURCES = AUTO
