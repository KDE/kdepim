INCLUDES = -I$(top_srcdir) -I$(top_builddir) $(all_includes)

kde_module_LTLIBRARIES = kcal_kabc.la

#libkcal_resourceremote_la_LDFLAGS = $(all_libraries) -no-undefined \
#                                    -version-info 1:0:0
#libkcal_resourceremote_la_LIBADD  = $(top_builddir)/libkcal/libkcal.la \
#                                    $(top_builddir)/libkdepim/libkdepim.la
#libkcal_resourceremote_la_SOURCES = resourceremote.cpp resourceremoteconfig.cpp


kcal_kabc_la_SOURCES = resourcekabc.cpp resourcekabcconfig.cpp
kcal_kabc_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kcal_kabc_la_LIBADD  = $(top_builddir)/libkcal/libkcal.la


#kcal_remote_la_SOURCES = resourceremote_plugin.cpp
#kcal_remote_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kcal_remote_la_LIBADD  = libkcal_resourceremote.la \
#                         $(top_builddir)/libkdepim/libkdepim.la

libkcalincludedir = $(includedir)/libkcal
libkcalinclude_HEADERS = resourcekabc.h resourcekabcconfig.h

servicedir = $(kde_servicesdir)/kresources/kcal
service_DATA = kabc.desktop

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kres_birthday.pot
