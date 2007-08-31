METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/kresources/scalix/shared -I$(top_srcdir) \
        -I$(top_builddir)/libkdepim $(all_includes)

# The scalix wizard links to this library too
lib_LTLIBRARIES = libkcalscalix.la

libkcalscalix_la_SOURCES = resourcescalix.cpp
libkcalscalix_la_LDFLAGS = $(all_libraries) -no-undefined
libkcalscalix_la_LIBADD = $(top_builddir)/libkcal/libkcal.la \
	$(top_builddir)/kresources/scalix/shared/libresourcescalixshared.la \
	-lkresources

kde_module_LTLIBRARIES = kcal_scalix.la

kcal_scalix_la_SOURCES = resourcescalix_plugin.cpp
kcal_scalix_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) -no-undefined
kcal_scalix_la_LIBADD = libkcalscalix.la

servicedir = $(kde_servicesdir)/kresources/kcal
service_DATA = scalix.desktop

install-data-local: $(srcdir)/../uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(servicedir)
	$(INSTALL_DATA) $(srcdir)/../uninstall.desktop $(DESTDIR)$(servicedir)/imap.desktop

