# $Id$

SUBDIRS = tests

METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/korganizer/interfaces \
    -I$(top_srcdir)/interfaces \
    -I$(top_srcdir)/libkcal -I$(top_builddir)/libkcal \
    -I$(top_srcdir)/libkpimexchange/core -I$(top_builddir)/libkpimexchange/core \
    -I$(top_srcdir)/libical/src/libical -I$(top_srcdir)/libical/src/libicalss \
    -I$(top_builddir)/libical/src/libical -I$(top_builddir)/libical/src/libicalss \
    $(all_includes)

noinst_LTLIBRARIES = libkpimexchangecalendar.la

libkpimexchangecalendar_la_SOURCES = exchangecalendar.cpp dateset.cpp
#libexchangecalendar_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0:0 -module
#libexchangecalendar_la_LIBADD = $(LIB_KIO) $(top_builddir)/libkcal/libkcal.la \
#    $(top_builddir)/libkpimexchange/libkpimexchange.la

kdepimincludedir = $(includedir)/kdepim
kdepiminclude_HEADERS = exchangecalendar.h
noinst_HEADERS = dateset.h

