### Makefile for the time conduit
###
### The time conduit is Copyright (C) 2002 by Reinhold Kainhofer

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
libtimeconduit_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libtimeconduit_la_LIBADD = ../../lib/libkpilot.la
 
