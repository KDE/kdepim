INCLUDES = -I$(top_srcdir)/kitchensync/kitchensync/lib/ \
  -I$(top_srcdir)/kitchensync/libksync \
  -I$(top_srcdir)/kitchensync/libkonnector2 \
  -I$(top_builddir)/libkdepim \
  -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libksync_backup.la libksync_restore.la

libksync_backup_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libksync_backup_la_LIBADD =  ../lib/libkitchensyncui.la
libksync_backup_la_SOURCES = backup.cpp backupview.cpp

libksync_restore_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libksync_restore_la_LIBADD =  ../lib/libkitchensyncui.la
libksync_restore_la_SOURCES = restore.cpp backupview.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kitchensync
service_DATA = backup.desktop restore.desktop
