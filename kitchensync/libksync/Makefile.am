SUBDIRS =  plugins .

lib_LTLIBRARIES= libkitchensync.la

INCLUDES = -I$(top_srcdir)/kitchensync/lib -I$(top_srcdir)/kitchensync/libkonnector -I$(top_srcdir) $(all_includes)

libkitchensync_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 0:0 -no-undefined
libkitchensync_la_LIBADD = $(top_builddir)/kitchensync/lib/libksyncentries.la  $(LIB_KDECORE) $(LIB_QT)  plugins/kalendar/libkitchensynckalendar.la
libkitchensync_la_SOURCES = ksync_plugin.cc ksync_return.cc ksync_sync.cc


#servicetype_DATA = konnector.desktop
#servicetypedir = $(kde_servicetypesdir)

METASOURCES = AUTO
include_HEADERS = ksync_plugin.h ksync_return.h ksync_sync.h
