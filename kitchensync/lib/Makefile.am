#SUBDIRS = test

lib_LTLIBRARIES= libksyncentries.la

INCLUDES = -I$(top_srcdir) $(all_includes)

libksyncentries_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 0:0 -no-undefined
libksyncentries_la_SOURCES = kontainer.cc kunknownsyncentry.cpp ksyncentry.cpp kalendarsyncentry.cpp kaddressbooksyncentry.cpp opiedesktopsyncentry.cpp
libksyncentries_la_LIBADD = $(LIB_QT)  $(top_builddir)/libkcal/libkcal.la -lkabc

#servicesdir = $(kde_servicesdir)/kded
#services_DATA = ksharedfile.desktop

METASOURCES = AUTO

include_HEADERS = kontainer.h ksyncentry.h kunknownsyncentry.h kalendarsyncentry.h kaddressbooksyncentry.h opiedesktopsyncentry.h
