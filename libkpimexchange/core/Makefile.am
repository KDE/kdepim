# $Id$

SUBDIRS=

METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/korganizer/interfaces -I$(top_srcdir) \
    -I$(top_srcdir)/libical/src/libical -I$(top_srcdir)/libical/src/libicalss \
    -I$(top_builddir)/libical/src/libical -I$(top_builddir)/libical/src/libicalss \
    $(all_includes)

noinst_LTLIBRARIES = libkpimexchangecore.la

libkpimexchangecore_la_SOURCES = exchangeclient.cpp exchangeaccount.cpp \
 			   exchangedownload.cpp exchangeupload.cpp exchangedelete.cpp \
			   utils.cpp exchangeprogress.cpp 
#libkpimexchange_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0:0 -module
#libkpimexchange_la_LIBADD = $(LIB_KIO) $(top_builddir)/libkcal/libkcal.la -lmimelib

kdepimincludedir = $(includedir)/kdepim
kdepiminclude_HEADERS = exchangeclient.h exchangeaccount.h

noinst_HEADERS = exchangedownload.h exchangeupload.h exchangedelete.h exchangeprogress.h utils.h

