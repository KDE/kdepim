#SUBDIRS = test

lib_LTLIBRARIES= libkonnector.la

INCLUDES = -I$(top_srcdir)/kitchensync/lib $(all_includes)

libkonnector_la_LDFLAGS = $(all_libraries) $(KDE_EXTRA_RPATH) $(KDE_RPATH) \
	-version-info 0:0 -no-undefined
libkonnector_la_LIBADD = $(top_builddir)/kitchensync/lib/libksync2.la
libkonnector_la_SOURCES = konnector.cpp kapabilities.cpp \
    kdevice.cpp konnectormanager.cpp \
    idhelper.cpp \
    konnectorinfo.cpp notify.cpp \
    error.cpp progress.cpp stdprogress.cpp stderror.cpp


servicetype_DATA = konnector.desktop
servicetypedir = $(kde_servicetypesdir)

METASOURCES = AUTO
SUBDIRS = .  plugins test
#include_HEADERS = koperations.h kapabilities.h kdevice.h konnectormanager.h konnector.h idhelper.h configpart.h error.h 
