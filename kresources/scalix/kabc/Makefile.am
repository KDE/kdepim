METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/kresources/scalix/shared -I$(top_srcdir) $(all_includes)

# The scalix wizard links to this library too
lib_LTLIBRARIES = libkabcscalix.la

libkabcscalix_la_SOURCES = resourcescalix.cpp contact.cpp
libkabcscalix_la_LDFLAGS = $(all_libraries) -no-undefined
libkabcscalix_la_LIBADD =  \
	$(top_builddir)/kresources/scalix/shared/libresourcescalixshared.la \
	-lkresources -lkabc

kde_module_LTLIBRARIES = kabc_scalix.la

noinst_HEADERS = resourcescalix.h

kabc_scalix_la_SOURCES = resourcescalix_plugin.cpp
kabc_scalix_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) -no-undefined
kabc_scalix_la_LIBADD = libkabcscalix.la

servicedir = $(kde_servicesdir)/kresources/kabc
service_DATA = scalix.desktop

install-data-local: $(srcdir)/../uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(servicedir)
	$(INSTALL_DATA) $(srcdir)/../uninstall.desktop $(DESTDIR)$(servicedir)/imap.desktop
