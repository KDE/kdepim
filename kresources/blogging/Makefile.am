INCLUDES = -I$(top_srcdir) -I$(top_builddir)/kresources/blogging/ $(all_includes)

noinst_LTLIBRARIES = libkpimblogging.la
libkpimblogging_la_SOURCES = api_blogger.cpp bloginterface.cpp xmlrpciface.cpp
# api_drupal.cpp api_file.cpp  api_kdedevelopers.cpp

kde_module_LTLIBRARIES = kcal_blogging.la

kcal_blogging_la_SOURCES = resourcebloggingsettings.ui kcal_resourcebloggingconfig.cpp \
    kcal_resourceblogging.cpp kcal_resourceblogging_plugin.cpp 
# kblog.cpp
kcal_blogging_la_LDFLAGS = -module $(KDE_PLUGIN)
kcal_blogging_la_LIBADD  = libkpimblogging.la \
                         $(top_builddir)/libkdepim/libkdepim.la

### libkcalincludedir = $(includedir)/libkcal
### libkcalinclude_HEADERS = resourcebloggingconfig.h resourceblogging.h

servicedir = $(kde_servicesdir)/kresources/kcal
service_DATA = blogging.desktop

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kres_blogging.pot

