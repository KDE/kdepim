### Makefile for the abbrowser / kabc conduit
###
### The abbrowser conduit is Copyright (C) 2001 by Greg Stern

INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = abbrowser_conduit.desktop

kde_module_LTLIBRARIES = libaddressconduit.la


libaddressconduit_la_SOURCES = \
	resolutionDialog.cc \
	abbrowser-factory.cc \
	abbrowser-setup.cc \
	abbrowser-conduit.cc \
	kaddressbookConduit.ui
libaddressconduit_la_LDFLAGS = $(KDE_RPATH) -L../../lib 
libaddressconduit_la_LIBADD = -lkpilot -lkabc
