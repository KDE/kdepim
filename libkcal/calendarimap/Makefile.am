METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/libkcal -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = kcal_imap.la 

kcal_imap_la_SOURCES = resourceimap.cpp resourceimap.skel \
                       resourceimapconfig.cpp kmailicalIface.stub
kcal_imap_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
kcal_imap_la_LIBADD = -lkresources $(top_builddir)/libkcal/libkcal.la

kmailicalIface_DIR = $(top_srcdir)/kmail

servicedir = $(kde_servicesdir)/kresources/kcal
service_DATA = imap.desktop
