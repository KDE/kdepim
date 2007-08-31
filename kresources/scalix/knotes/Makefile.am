METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/kresources/scalix/shared \
	-I$(top_srcdir) -I$(top_srcdir)/knotes -I$(top_builddir)/libkdepim $(all_includes)

# The scalix wizard links to this library too
lib_LTLIBRARIES = libknotesscalix.la

libknotesscalix_la_SOURCES = resourcescalix.cpp
libknotesscalix_la_LDFLAGS = $(all_libraries) -no-undefined
libknotesscalix_la_LIBADD = \
	$(top_builddir)/kresources/scalix/shared/libresourcescalixshared.la \
	$(top_builddir)/knotes/libknotesresources.la \
	$(top_builddir)/libkcal/libkcal.la \
	-lkresources -lkdeprint

kde_module_LTLIBRARIES = knotes_scalix.la

knotes_scalix_la_SOURCES = resourcescalix_plugin.cpp
knotes_scalix_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) -no-undefined
knotes_scalix_la_LIBADD = libknotesscalix.la

servicedir = $(kde_servicesdir)/kresources/knotes
service_DATA = scalix.desktop

install-data-local: $(srcdir)/../uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(servicedir)
	$(INSTALL_DATA) $(srcdir)/../uninstall.desktop $(DESTDIR)$(servicedir)/scalix.desktop
