METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/libkcal -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = resourcecalendarimap.la 

resourcecalendarimap_la_SOURCES = resourceimap.cpp resourceimap.skel \
	resourceimapconfig.cpp kmailicalIface.stub
resourcecalendarimap_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
resourcecalendarimap_la_LIBADD= -lkresources \
                                $(top_builddir)/libkcal/libkcal.la

kmailicalIface_DIR = $(top_srcdir)/kmail

linkdir= $(kde_datadir)/kresources/calendar
link_DATA= imap.desktop
