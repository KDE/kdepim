INCLUDES = -I$(top_srcdir)/kitchensync/kitchensync/lib -I$(top_srcdir)/kitchensync/libksync \
		-I$(top_srcdir)/kitchensync/libkonnector2 \
		-I$(top_srcdir) -I$(top_srcdir)/libkcal $(all_includes)

kde_module_LTLIBRARIES = liborganizerpart.la
liborganizerpart_la_LDFLAGS = $(all_libraries) $(LIB_KPARTS) $(KDE_PLUGIN)
liborganizerpart_la_LIBADD =  ../lib/libkitchensyncui.la 

liborganizerpart_la_SOURCES = organizerpart.cpp organizerbase.ui

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = organizer.desktop

