### Makefile for the avantgo conduit
###
### The mal conduit is Copyright (C) 2002 by Reinhold Kainhofer


INCLUDES= $(PISOCK_INCLUDE)  $(MAL_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = mal_conduit.desktop

kde_module_LTLIBRARIES = conduit_mal.la

conduit_mal_la_SOURCES = mal-setup_dialog.ui mal-factory.cc mal-setup.cc mal-conduit.cc 
conduit_mal_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
conduit_mal_la_LIBADD = ../../lib/libkpilot.la $(MAL_LIB) $(LIB_KDEUI)

