INCLUDES = -I$(top_srcdir)/kitchensync/lib -I$(top_srcdir)/kitchensync/libkonnector $(all_includes)

lib_LTLIBRARIES = libkitchensync.la

libkitchensync_la_SOURCES = partbar.cc manipulatorpart.cc ksync_configuredialog.cc ksync_systemtray.cc ksync_mainwindow.cc ksync_configpart.cc
libkitchensync_la_LDFLAGS = $(all_libraries) -no-undefined 
libkitchensync_la_LIBADD = $(LIB_KPARTS) $(top_builddir)/kitchensync/libkonnector/libkonnector.la

ksyncincludedir = $(includedir)/kitchensync
ksyncinclude_HEADERS = partbar.h manipulatorpart.h ksync_systemtray.h ksync_mainwindow.h

METASOURCES = AUTO

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kitchensync.desktop
