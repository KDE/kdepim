# $Id$

SUBDIRS=

METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/korganizer/interfaces -I$(top_srcdir) -I$(top_builddir)/libkdepim \
    -I$(top_srcdir)/libkcal/libical/src/libical -I$(top_srcdir)/libkcal/libical/src/libicalss \
    -I$(top_builddir)/libkcal/libical/src/libical -I$(top_builddir)/libkcal/libical/src/libicalss \
    $(all_includes)

#     -I$(top_builddir)/libkdepim/resources -I$(top_builddir)/libkdepim/resources/calendar

noinst_LTLIBRARIES = libkpimexchangecore.la

libkpimexchangecore_la_SOURCES = exchangeclient.cpp exchangeaccount.cpp \
			   exchangedownload.cpp exchangeupload.cpp exchangedelete.cpp \
			   utils.cpp exchangeprogress.cpp exchangemonitor.cpp 

#libkpimexchange_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0:0 -module
#libkpimexchange_la_LIBADD = $(LIB_KIO) $(top_builddir)/libkcal/libkcal.la

kdepimincludedir = $(includedir)/kdepim
kdepiminclude_HEADERS = exchangeclient.h exchangeaccount.h

noinst_HEADERS = exchangedownload.h exchangeupload.h exchangedelete.h exchangeprogress.h utils.h

