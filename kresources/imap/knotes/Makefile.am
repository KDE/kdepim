METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/kresources/imap/shared \
	-I$(top_srcdir)/knotes $(all_includes)

kde_module_LTLIBRARIES = knotes_imap.la

noinst_HEADERS = resourceimap.h

knotes_imap_la_SOURCES = resourceimap.cpp
knotes_imap_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
knotes_imap_la_LIBADD = -lkresources -lkdeprint \
	$(top_builddir)/kresources/imap/shared/libresourceimapshared.la \
	$(top_builddir)/knotes/libknotes.la \
	$(top_builddir)/libkcal/libkcal.la

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/knotes_imap.pot

servicedir = $(kde_servicesdir)/kresources/knotes
service_DATA = imap.desktop
