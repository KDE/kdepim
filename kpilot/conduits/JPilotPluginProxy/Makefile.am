###
### Makefile for JPilot Plugin conduits. These two conduits share most code,
### so it seems logical to put them in one place.
###

INCLUDES= -I$(top_srcdir)/kpilot/lib -I/usr/include/gtk-1.2 -I/usr/include/glib-1.2/ -I/usr/lib/glib/include $(all_includes)

METASOURCES = AUTO
servicedir = $(kde_servicesdir)
service_DATA = JPilotPluginProxy.desktop

kde_module_LTLIBRARIES = libJPilotProxy.la

libJPilotProxy_la_SOURCES = JPilotPluginProxy.ui \
	JPilotProxy-conduit.cc JPilotProxy-factory.cc JPilotProxy-setup.cc jplugin.cpp libplugin.c
libJPilotProxy_la_LDFLAGS = $(KDE_RPATH) -L../../lib -Wl,-export-dynamic
libJPilotProxy_la_LIBADD = -lkpilot

