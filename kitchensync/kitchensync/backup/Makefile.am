INCLUDES = -I$(top_srcdir)/kitchensync/kitchensync/lib/ \
  -I$(top_srcdir)/kitchensync/lib/ \
  -I$(top_srcdir)/kitchensync/libkonnector2 \
  -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libksync_backup.la

libksync_backup_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libksync_backup_la_LIBADD =  ../lib/libkitchensyncui.la
libksync_backup_la_SOURCES = backup.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kitchensync
service_DATA = backup.desktop
