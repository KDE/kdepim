### Makefile for the abbrowser / kabc conduit.
### 	conduits/abbrowserconduit
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
libaddressconduit_la_LDFLAGS = $(KDE_RPATH) $(KDE_PLUGIN) $(all_libraries)

## The LIB_KDEUI is superfluous, because kabc pulls it in anyway,
## but to be consistent (and in case kabc changes behavior) we'll
## add it anyway.
libaddressconduit_la_LIBADD = ../../lib/libkpilot.la -lkabc $(LIB_KDEUI)
