METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/kresources/kolab/shared -I$(top_srcdir) $(all_includes)

# The kolab wizard links to this library too
lib_LTLIBRARIES = libkabckolab.la

libkabckolab_la_SOURCES = resourcekolab.cpp contact.cpp
libkabckolab_la_LDFLAGS = $(all_libraries) -no-undefined
libkabckolab_la_LIBADD =  \
	$(top_builddir)/kresources/kolab/shared/libresourcekolabshared.la \
	-lkresources -lkabc

kde_module_LTLIBRARIES = kabc_kolab.la

noinst_HEADERS = resourcekolab.h contact.h

kabc_kolab_la_SOURCES = resourcekolab_plugin.cpp
kabc_kolab_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) -no-undefined
kabc_kolab_la_LIBADD = libkabckolab.la

servicedir = $(kde_servicesdir)/kresources/kabc
service_DATA = kolab.desktop

install-data-local: $(srcdir)/../uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(servicedir)
	$(INSTALL_DATA) $(srcdir)/../uninstall.desktop $(DESTDIR)$(servicedir)/imap.desktop
