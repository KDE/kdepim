###
### Makefile for JPilot Plugin conduits. These two conduits share most code,
### so it seems logical to put them in one place.
###

INCLUDES= -I$(top_srcdir)/kpilot/lib $(GLIB_INCLUDE) $(GTK_INCLUDE) $(all_includes)

METASOURCES = AUTO
SUBDIRS = JPilotAPI

servicedir = $(kde_servicesdir)
service_DATA = JPilotPluginProxy.desktop

kde_module_LTLIBRARIES = libJPilotProxy.la

libJPilotProxy_la_SOURCES = ConduitDialog.ui JPilotProxy-conduit.cc JPilotProxy-factory.cc JPilotProxy-setup.cc jplugin.cpp
libJPilotProxy_la_LDFLAGS = $(KDE_PLUGIN) $(KDE_RPATH)
libJPilotProxy_la_LIBADD = ../../lib/libkpilot.la -lgtk -lgdbm -lgdk

