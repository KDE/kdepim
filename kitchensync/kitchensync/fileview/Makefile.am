INCLUDES = -I$(top_srcdir)/kitchensync/kitchensync/lib/ -I$(top_srcdir)/kitchensync/libksync $(all_includes)

kde_module_LTLIBRARIES = libfileviewpart.la
libfileviewpart_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libfileviewpart_la_LIBADD =  ../lib/libkitchensyncui.la 

libfileviewpart_la_SOURCES = ksync_fileviewpart.cc \
			     ksync_fileviewwidget.cc

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = fileview.desktop

