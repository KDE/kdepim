INCLUDES = -I$(top_srcdir)/kitchensync/kitchensync/lib/ \
  -I$(top_srcdir)/kitchensync/libksync \
  -I$(top_srcdir)/kitchensync/libkonnector2 \
  -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libksync_viewer.la

libksync_viewer_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libksync_viewer_la_LIBADD =  ../lib/libkitchensyncui.la
libksync_viewer_la_SOURCES = viewer.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kitchensync
service_DATA = viewer.desktop
