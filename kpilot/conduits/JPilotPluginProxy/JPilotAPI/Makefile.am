###
### Makefile for JPilot Plugin conduits. These two conduits share most code,
### so it seems logical to put them in one place.
###

INCLUDES= -I$(top_srcdir)/kpilot/lib $(GLIB_INCLUDE) $(GTK_INCLUDE)  $(all_includes)

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
kde_module_LTLIBRARIES = libJPilotAPI.la 

libJPilotAPI_la_SOURCES = libplugin.c APIEmulation.c
libJPilotAPI_la_LDFLAGS = $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries)
libJPilotAPI_la_LIBADD = -lc -lgtk -lgdk

