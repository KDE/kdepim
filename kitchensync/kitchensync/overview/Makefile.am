INCLUDES = -I$(top_srcdir)/kitchensync/kitchensync/lib/ -I$(top_srcdir)/kitchensync/lib/ \
		-I$(top_srcdir)/kitchensync/libkonnector $(all_includes)

kde_module_LTLIBRARIES = liboverviewpart.la
liboverviewpart_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
liboverviewpart_la_LIBADD =  ../lib/libkitchensyncui.la

liboverviewpart_la_SOURCES = overviewwidget.cc ksync_overviewpart.cc

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = overview.desktop

