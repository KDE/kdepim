# $Id$

SUBDIRS =

METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/libkcal \
    $(all_includes)

kde_module_LTLIBRARIES = resourcecalendarimap.la 

resourcecalendarimap_la_SOURCES = resourceimap.cpp resourceimapconfig.cpp
resourcecalendarimap_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
resourcecalendarimap_la_LIBADD= -lkderesources \
                                $(top_builddir)/libkcal/libkcal.la 

linkdir= $(kde_datadir)/resources/calendar
link_DATA= imap.desktop
