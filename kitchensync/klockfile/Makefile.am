SUBDIRS = test

kde_module_LTLIBRARIES = libkded_ksharedfile.la
lib_LTLIBRARIES= libksharedfile.la

INCLUDES = -I$(top_builddir)/libkdepim $(all_includes)
libkded_ksharedfile_la_SOURCES = kdedsharedfile.cpp kdedsharedfile.skel
libkded_ksharedfile_la_LDFLAGS = $(all_libraries) -module -avoid-version
libkded_ksharedfile_la_LIBADD = $(LIB_KSYCOCA)

libksharedfile_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 0:0 -no-undefined
libksharedfile_la_LIBADD = $(LIB_KDECORE) $(LIB_KDEUI) -lDCOP $(LIB_QT) 
libksharedfile_la_SOURCES = ksharedfile.cpp ksharedfile.skel ksharedfiledevice.cpp


servicesdir = $(kde_servicesdir)/kded
services_DATA = ksharedfile.desktop

METASOURCES = AUTO

include_HEADERS = ksharedfile.h

