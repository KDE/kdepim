### Makefile for the abbrowser / kabc conduit
###
### The abbrowser conduit is Copyright (C) 2001 by Greg Stern

INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = abbrowser_conduit.desktop

kde_module_LTLIBRARIES = libabbrowserconduit.la


libabbrowserconduit_la_SOURCES = \
	abbrowser-factory.cc \
	abbrowser-setup.cc \
	abbrowser-conduit.cc \
	abbrowserConduitConfig_base.ui
libabbrowserconduit_la_LDFLAGS = $(KDE_RPATH) -L../../lib
libabbrowserconduit_la_LIBADD = ../../lib/libkpilot.la
# You may want to enable debugging like this on systems with broken kdelibs
#
libabbrowserconduit_la_CFLAGS = -DDEBUG -DDEBUG_CERR

