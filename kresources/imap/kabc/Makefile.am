METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/kresources/imap/shared $(all_includes)

kde_module_LTLIBRARIES = kabc_imap.la

noinst_HEADERS = resourceimap.h

kabc_imap_la_SOURCES = resourceimap.cpp
kabc_imap_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kabc_imap_la_LIBADD = -lkresources -lkabc \
	$(top_builddir)/kresources/imap/shared/libresourceimapshared.la

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabc_imap.pot

servicedir = $(kde_servicesdir)/kresources/kabc
service_DATA = imap.desktop
