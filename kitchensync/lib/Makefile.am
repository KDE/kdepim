#SUBDIRS = test

lib_LTLIBRARIES= libksyncentries.la

INCLUDES = $(all_includes)

libksyncentries_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 0:0 -no-undefined
libksyncentries_la_SOURCES = kunknownsyncentry.cpp ksyncentry.cpp kalendarsyncentry.cpp kaddressbooksyncentry.cpp
libksyncentries_la_LIBADD = $(LIB_QT)  -lkcal -lkabc

#servicesdir = $(kde_servicesdir)/kded
#services_DATA = ksharedfile.desktop

METASOURCES = AUTO

include_HEADERS = ksyncentry.h kunknownsyncentry.h kalendarsyncentry.h kaddressbooksyncentry.h