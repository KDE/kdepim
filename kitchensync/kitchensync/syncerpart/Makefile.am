INCLUDES = -I$(top_srcdir)/kitchensync/kitchensync/lib \
  -I$(top_srcdir)/kitchensync/libksync \
  -I$(top_srcdir)/kitchensync/libkonnector2 \
  -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libksync_syncerpart.la

libksync_syncerpart_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libksync_syncerpart_la_LIBADD =  ../lib/libkitchensyncui.la
libksync_syncerpart_la_SOURCES = syncerpart.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kitchensync
service_DATA = syncerpart.desktop
