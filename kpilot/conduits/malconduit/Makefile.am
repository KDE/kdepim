### Makefile for the avantgo conduit
###
### The mal conduit is Copyright (C) 2002 by Reinhold Kainhofer


INCLUDES= $(PISOCK_INCLUDE)  $(MAL_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = mal_conduit.desktop

kde_module_LTLIBRARIES = libmalconduit.la
libmalconduit_la_SOURCES = mal-setup_dialog.ui mal-factory.cc mal-setup.cc mal-conduit.cc 
	
libmalconduit_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libmalconduit_la_LIBADD = ../../lib/libkpilot.la $(MAL_LIB) $(LIB_KDEUI)

