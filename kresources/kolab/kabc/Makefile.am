METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/kresources/kolab/shared -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = kabc_kolab.la

noinst_HEADERS = resourcekolab.h

kabc_kolab_la_SOURCES = resourcekolab.cpp
kabc_kolab_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kabc_kolab_la_LIBADD =  $(top_builddir)/libkcal/libkcal.la \
	$(top_builddir)/kresources/kolab/shared/libresourcekolabshared.la \
	$(top_builddir)/libkdepim/libkdepim.la -lkresources -lkabc
messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabc_kolab.pot

servicedir = $(kde_servicesdir)/kresources/kabc
service_DATA = kolab.desktop
