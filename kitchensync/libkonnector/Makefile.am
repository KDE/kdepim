#SUBDIRS = test

lib_LTLIBRARIES= libksharedfile.la

INCLUDES = $(all_includes) -I$(top_srcdir)/kitchensync/lib

libksharedfile_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 0:0 -no-undefined
libksharedfile_la_LIBADD = $(top_builddir)/kitchensync/lib/libksyncentries.la  $(LIB_KDECORE) $(LIB_QT) 
libksharedfile_la_SOURCES = konnectorplugin.cpp kapabilities.cpp kdevice.cpp konnector.cpp


#servicesdir = $(kde_servicesdir)/kded
#services_DATA = ksharedfile.desktop

METASOURCES = AUTO

include_HEADERS = kapabilities.h kdevice.h konnector.h konnectorplugin.h