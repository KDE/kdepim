METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/kresources/imap/shared $(all_includes)

kde_module_LTLIBRARIES = knotes_imap.la

noinst_HEADERS = resourceimap.h

knotes_imap_la_SOURCES = resourceimap.cpp resourceimap.skel kmailicalIface.stub
knotes_imap_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
knotes_imap_la_LIBADD = -lkresources \
	$(top_builddir)/kresources/imap/shared/libresourceimapshared.la

kmailicalIface_DIR = $(top_srcdir)/kmail

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/knotes_imap.pot

servicedir = $(kde_servicesdir)/kresources/knotes
service_DATA = imap.desktop
