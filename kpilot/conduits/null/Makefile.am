INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = null-conduit.desktop

kde_module_LTLIBRARIES = libnullconduit.la


libnullconduit_la_SOURCES = setup_base.ui null-conduit.cc null-factory.cc
libnullconduit_la_LDFLAGS = $(KDE_RPATH) $(KDE_PLUGIN)
libnullconduit_la_LIBADD = ../../lib/libkpilot.la $(LIB_KDEUI)

