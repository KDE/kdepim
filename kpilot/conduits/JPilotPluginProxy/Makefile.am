### Makefile for JPilot Plugin conduits. 
### 	conduits/JPilotPluginProxy
###
### This conduit is deprecated while we figure out how to put
### JPilot widgets into the Qt3 event loop.
###

INCLUDES= -I$(top_srcdir)/kpilot/lib $(GLIB_INCLUDE) $(GTK_INCLUDE) $(all_includes)

METASOURCES = AUTO
SUBDIRS = JPilotAPI

servicedir = $(kde_servicesdir)
service_DATA = JPilotPluginProxy.desktop

kde_module_LTLIBRARIES = libJPilotProxy.la

libJPilotProxy_la_SOURCES = ConduitDialog.ui JPilotProxy-conduit.cc \
	JPilotProxy-factory.cc JPilotProxy-setup.cc jplugin.cpp
libJPilotProxy_la_LDFLAGS = $(KDE_RPATH) $(KDE_PLUGIN) $(all_libraries) -Wl,-export-dynamic
libJPilotProxy_la_LIBADD = ../../lib/libkpilot.la -lgtk -lgdbm -lgdk

