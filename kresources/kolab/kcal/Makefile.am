METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/kresources/kolab/shared -I$(top_srcdir) \
        -I$(top_builddir)/libkdepim $(all_includes)

# The kolab wizard links to this library too
lib_LTLIBRARIES = libkcalkolab.la

libkcalkolab_la_SOURCES = incidence.cpp event.cpp task.cpp journal.cpp resourcekolab.cpp
libkcalkolab_la_LDFLAGS = $(all_libraries) -no-undefined
libkcalkolab_la_LIBADD = $(top_builddir)/libkcal/libkcal.la \
	$(top_builddir)/kresources/kolab/shared/libresourcekolabshared.la \
	-lkresources

kde_module_LTLIBRARIES = kcal_kolab.la

kcal_kolab_la_SOURCES = resourcekolab_plugin.cpp
kcal_kolab_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) -no-undefined
kcal_kolab_la_LIBADD = libkcalkolab.la

servicedir = $(kde_servicesdir)/kresources/kcal
service_DATA = kolab.desktop

install-data-local: $(srcdir)/../uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(servicedir)
	$(INSTALL_DATA) $(srcdir)/../uninstall.desktop $(DESTDIR)$(servicedir)/imap.desktop

