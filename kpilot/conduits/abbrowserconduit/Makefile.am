### Makefile for the abbrowser / kabc conduit
###
### The abbrowser conduit is Copyright (C) 2001 by Greg Stern
### Copyright (C) 2002 by Reinhold Kainhofer

INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = abbrowser_conduit.desktop

kde_module_LTLIBRARIES = libaddressconduit.la

# uncomment the following line to enable the addressbook conduit. 
# THIS IS DANGEROUS AND MIGHT CAUSE DATA LOSS as the conduit 
# has not been tested much!!!
#
# The address book conduit was deemed "safe" before KDE 3.1 came out.
KDE_CXXFLAGS=-DDO_DANGEROUS_ABOOK_SYNC

libaddressconduit_la_SOURCES = \
	resolutionDialog.cc \
	abbrowser-factory.cc \
	abbrowser-setup.cc \
	abbrowser-conduit.cc \
	kaddressbookConduit.ui
libaddressconduit_la_LDFLAGS = $(KDE_RPATH) -L../../lib $(all_libraries)
libaddressconduit_la_LIBADD = -lkpilot -lkabc


#bin_PROGRAMS = test
#
#test_SOURCES = test.cpp
#test_LDFLAGS = $(all_libraries) $(KDE_RPATH) -L../../lib
#test_LDADD = $(LIB_KFILE) 

