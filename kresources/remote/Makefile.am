INCLUDES = -I$(top_srcdir) $(all_includes) -I$(top_builddir)/libkdepim

lib_LTLIBRARIES   = libkcal_resourceremote.la
libkcal_resourceremote_la_LDFLAGS = $(all_libraries) -no-undefined \
                                    -version-info 1:0:0
libkcal_resourceremote_la_LIBADD  = $(top_builddir)/libkcal/libkcal.la \
                                    $(top_builddir)/libkdepim/libkdepim.la
libkcal_resourceremote_la_SOURCES = resourceremote.cpp resourceremoteconfig.cpp

kde_module_LTLIBRARIES = kcal_remote.la

kcal_remote_la_SOURCES = resourceremote_plugin.cpp
kcal_remote_la_LDFLAGS = -module $(KDE_PLUGIN)
kcal_remote_la_LIBADD  = libkcal_resourceremote.la \
                         $(top_builddir)/libkdepim/libkdepim.la

libkcalincludedir = $(includedir)/libkcal
libkcalinclude_HEADERS = resourceremoteconfig.h resourceremote.h

servicedir = $(kde_servicesdir)/kresources/kcal
service_DATA = remote.desktop

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kres_remote.pot
