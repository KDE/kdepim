METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/kresources/kolab/shared \
	-I$(top_srcdir) -I$(top_srcdir)/knotes $(all_includes)

kde_module_LTLIBRARIES = knotes_kolab.la

noinst_HEADERS = resourcekolab.h note.h

knotes_kolab_la_SOURCES = resourcekolab.cpp note.cpp
knotes_kolab_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
knotes_kolab_la_LIBADD = -lkresources -lkdeprint \
	$(top_builddir)/kresources/kolab/shared/libresourcekolabshared.la \
	$(top_builddir)/knotes/libknotes.la \
	$(top_builddir)/libkcal/libkcal.la

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/knotes_kolab.pot

servicedir = $(kde_servicesdir)/kresources/knotes
service_DATA = kolabresource.desktop
