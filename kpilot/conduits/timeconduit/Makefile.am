### Makefile for the time conduit
###
### The time conduit is Copyright (C) 2002 by Reinhold Kainhofer

INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = time_conduit.desktop

kde_module_LTLIBRARIES = conduit_time.la


conduit_time_la_SOURCES = \
	time-factory.cc \
	time-setup.cc \
	time-conduit.cc \
	time-setup_dialog.ui
conduit_time_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
conduit_time_la_LIBADD = ../../lib/libkpilot.la $(LIB_KDEUI)
 
