### Makefile for the abbrowser / kabc conduit
###
### The abbrowser conduit is Copyright (C) 2001 by Greg Stern

INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = time_conduit.desktop

kde_module_LTLIBRARIES = libtimeconduit.la


libtimeconduit_la_SOURCES = \
	time-factory.cc \
	time-setup.cc \
	time-conduit.cc \
	time-setup_dialog.ui
libtimeconduit_la_LDFLAGS = $(KDE_RPATH) -L../../lib 
libtimeconduit_la_LIBADD = -lkpilot 
