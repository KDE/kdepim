INCLUDES = -I$(top_srcdir)/kitchensync/kitchensync/lib/ \
  -I$(top_srcdir)/kitchensync/libksync \
  -I$(top_srcdir)/kitchensync/libkonnector2 \
  -I$(top_builddir)/libkdepim \
  -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libksync_debugger.la

libksync_debugger_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libksync_debugger_la_LIBADD =  ../lib/libkitchensyncui.la
libksync_debugger_la_SOURCES = debugger.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kitchensync
service_DATA = debugger.desktop
