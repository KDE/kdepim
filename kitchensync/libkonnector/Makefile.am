lib_LTLIBRARIES= libkonnector.la

INCLUDES = -I$(top_srcdir)/kitchensync/lib $(all_includes)

libkonnector_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 0:0 -no-undefined
libkonnector_la_LIBADD = $(top_builddir)/kitchensync/lib/libksync.la  $(LIB_KDECORE) $(LIB_QT) 
libkonnector_la_SOURCES = konnectorplugin.cpp kapabilities.cpp \
    kdevice.cpp konnector.cpp \
    koperations.cpp idhelper.cc \
    configwidget.cpp configpart.cpp


servicetype_DATA = konnector.desktop
servicetypedir = $(kde_servicetypesdir)

METASOURCES = AUTO
SUBDIRS = .  plugins
include_HEADERS = koperations.h kapabilities.h kdevice.h konnector.h konnectorplugin.h idhelper.h
