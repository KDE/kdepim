INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = abbrowser_conduit.desktop

kde_module_LTLIBRARIES = libaddressconduit.la

libaddressconduit_la_SOURCES = resolutionDialog.cc abbrowser-factory.cc abbrowser-setup.cc abbrowser-conduit.cc kaddressbookConduit.ui
libaddressconduit_la_LDFLAGS = $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries)
libaddressconduit_la_LIBADD = ../../lib/libkpilot.la -lkabc
