###
### Makefile for JPilot Plugin conduits. These two conduits share most code,
### so it seems logical to put them in one place.
###

INCLUDES= -I$(top_srcdir)/kpilot/lib $(GLIB_INCLUDE) $(GTK_INCLUDE) $(all_includes)

METASOURCES = AUTO
SUBDIRS = JPilotAPI

servicedir = $(kde_servicesdir)
service_DATA = JPilotPluginProxy.desktop

kde_module_LTLIBRARIES = conduit_JPilotProxy.la

conduit_JPilotProxy_la_SOURCES = jpilotproxysettings.kcfgc ConduitDialog.ui JPilotProxy-conduit.cc JPilotProxy-factory.cc JPilotProxy-setup.cc jplugin.cpp
conduit_JPilotProxy_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
conduit_JPilotProxy_la_LIBADD = ../../lib/libkpilot.la -lgtk -lgdbm -lgdk

kde_kcfg_DATA = jpilotproxysettings.kcfg
