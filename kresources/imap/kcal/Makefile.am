METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/libkcal -I$(top_srcdir)/kresources/imap/shared \
	-I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = kcal_imap.la

kcal_imap_la_SOURCES = resourceimap.cpp resourceimapconfig.cpp
kcal_imap_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
kcal_imap_la_LIBADD = -lkresources \
	$(top_builddir)/kresources/imap/shared/libresourceimapshared.la \
	$(top_builddir)/libkcal/libkcal.la

servicedir = $(kde_servicesdir)/kresources/kcal
service_DATA = imap.desktop
