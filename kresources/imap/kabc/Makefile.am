METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/kresources/imap/shared -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libkabc_imap.la kabc_imap.la

noinst_HEADERS = resourceimap.h

libkabc_imap_la_SOURCES = resourceimap.cpp
libkabc_imap_la_LDFLAGS = $(all_libraries)
libkabc_imap_la_LIBADD = -lkresources -lkabc \
	$(top_builddir)/kresources/imap/shared/libresourceimapshared.la \
	$(top_builddir)/libkdepim/libkdepim.la
messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabc_imap.pot

kabc_imap_la_SOURCES = resourceimap_plugin.cpp
kabc_imap_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kabc_imap_la_LIBADD = libkabc_imap.la

servicedir = $(kde_servicesdir)/kresources/kabc
service_DATA = imap.desktop
