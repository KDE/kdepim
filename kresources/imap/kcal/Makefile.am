METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/libkcal -I$(top_srcdir)/kresources/imap/shared \
	-I$(top_srcdir) $(all_includes)

lib_LTLIBRARIES   = libkcal_imap.la

libkcal_imap_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0:0
libkcal_imap_la_LIBADD  = -lkresources \
	$(top_builddir)/kresources/imap/shared/libresourceimapshared.la \
	$(top_builddir)/libkcal/libkcal.la
libkcal_imap_la_SOURCES = resourceimap.cpp resourceimapconfig.cpp

noinst_HEADERS = resourceimap.h resourceimapconfig.h

kde_module_LTLIBRARIES = kcal_imap.la

kcal_imap_la_SOURCES = resourceimap_plugin.cpp
kcal_imap_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
kcal_imap_la_LIBADD = libkcal_imap.la

servicedir = $(kde_servicesdir)/kresources/kcal
service_DATA = imap.desktop
