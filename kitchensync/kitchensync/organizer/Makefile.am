INCLUDES = -I$(top_srcdir)/kitchensync/kitchensync/lib/ $(all_includes)

kde_module_LTLIBRARIES = liborganizerpart.la
liborganizerpart_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
liborganizerpart_la_LIBADD =  ../lib/libkitchensync.la

liborganizerpart_la_SOURCES = ksync_organizerpart.cc

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = organizer.desktop

