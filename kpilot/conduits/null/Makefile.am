INCLUDES= $(all_includes) -I$(top_srcdir)/kpilot/lib 

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = null-conduit.desktop

kde_module_LTLIBRARIES = libnullconduit.la


libnullconduit_la_SOURCES = null-factory.cc setup_base.ui
libnullconduit_la_LDFLAGS = $(KDE_RPATH) -L../../lib
libnullconduit_la_LIBADD = -lkpilot

