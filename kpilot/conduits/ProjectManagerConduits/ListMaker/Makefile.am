### Makefile.am for kpilot/conduits/ProjectManagerConduits/ListMaker
###
### $Id$
###
###
### Makefile for the ListMaker conduit.
###

INCLUDES= -I$(top_srcdir)/kpilot/conduits/ProjectManagerConduits/lib -I$(top_srcdir)/kpilot/lib -I$(top_srcdir)/libkcal $(all_includes)

METASOURCES = AUTO
servicedir = $(kde_servicesdir)
service_DATA = ListMaker-conduit.desktop

### If you must get debugging output on a platform where
### the libs are built without debugging support, define
### DEBUG_CERR. Define DEBUG to get debugging support anywhere.
###
### KDE_CXXFLAGS=-DDEBUG -DDEBUG_CERR
#KDE_CXXFLAGS=-DDEBUG -DDEBUG_CERR


###SUBDIRS = ListMaker
###   Bonsai 
###          BrainForest 
###          ListMaker 
###          Progect 
###          ShadowPlan


kde_module_LTLIBRARIES = libListMakerConduit.la

libListMakerConduit_la_SOURCES = \
   ListMaker-conduit.cc  pilotListMakerEntry.cc ListMaker-factory.cc \
   ListMaker-setup.cc
libListMakerConduit_la_LDFLAGS = $(KDE_RPATH) -L$(top_srcdir)/kpilot/lib -L$(top_srcdir)/libkcal -L$(top_srcdir)/kpilot/conduits/ProjectManagerConduits/lib
libListMakerConduit_la_LIBADD = -lkpilot -lkcal -lProjectsConduit

