INCLUDES = -I$(top_srcdir) -I$(top_builddir) \
           -I$(top_builddir)/kaddressbook/common \
           -I$(top_srcdir)/kaddressbook/common \
             $(all_includes)

kde_module_LTLIBRARIES = kcal_kabc.la

kcal_kabc_la_SOURCES = resourcekabc.cpp resourcekabcconfig.cpp
kcal_kabc_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kcal_kabc_la_LIBADD  = $(top_builddir)/libkcal/libkcal.la \
                       $(top_builddir)/kaddressbook/common/libkabcommon.la

libkcalincludedir = $(includedir)/libkcal
libkcalinclude_HEADERS = resourcekabc.h resourcekabcconfig.h

servicedir = $(kde_servicesdir)/kresources/kcal
service_DATA = kabc.desktop

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kres_birthday.pot

resourcekabcconfig.lo: ../../kaddressbook/common/kabprefs_base.h
