INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkitchensync.la

libkitchensync_la_SOURCES = partbar.cc manipulatorpart.cc ksync_configuredialog.cc
libkitchensync_la_LDFLAGS = $(all_libraries) -no-undefined 
libkitchensync_la_LIBADD = $(LIB_KPARTS)

ksyncincludedir = $(includedir)/kitchensync
ksyncinclude_HEADERS = partbar.h manipulatorpart.h

METASOURCES = AUTO

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kitchensync.desktop
