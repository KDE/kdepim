### Makefile for the abbrowser / kabc conduit
###
### The abbrowser conduit is Copyright (C) 2001 by Greg Stern

INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = abbrowser_conduit.desktop

kde_module_LTLIBRARIES = libabbrowserconduit.la
noinst_PROGRAMS = testdcop


libabbrowserconduit_la_SOURCES = \
	contactentry.cpp \
	abbrowser-factory.cc \
	abbrowser-setup.cc \
	abbrowser-conduit.cc \
	abbrowserConduitConfig_base.ui
libabbrowserconduit_la_LDFLAGS = $(KDE_RPATH) -L../../lib
libabbrowserconduit_la_LIBADD = ../../lib/libkpilot.la
# You may want to enable debugging like this on systems with broken kdelibs
#
libabbrowserconduit_la_CFLAGS = -DDEBUG -DDEBUG_CERR

testdcop_SOURCES = testAbbrowserDCOP.cc
testdcop_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testdcop_LDADD = contactentry.lo $(LIB_KDECORE) $(LIB_KFILE)


