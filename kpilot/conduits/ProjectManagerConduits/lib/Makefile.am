### Makefile.am for kpilot/conduits/ProjectManagerConduits
###
### $Id$
###
###
### Makefile for the Project manager conduit lib.
###

INCLUDES= -I$(top_srcdir)/kpilot/lib -I$(top_srcdir)/libkcal \
	$(PISOCK_INCLUDE) $(all_includes)

METASOURCES = AUTO

### If you must get debugging output on a platform where
### the libs are built without debugging support, define
### DEBUG_CERR. Define DEBUG to get debugging support anywhere.
###
### KDE_CXXFLAGS=-DDEBUG -DDEBUG_CERR



lib_LTLIBRARIES = libProjectsConduit.la 

libProjectsConduit_la_SOURCES = DatabaseAction.cc DatabaseActionDialog.ui \
	DatabaseActiondlgPrivate.cpp MultiDB-conduit.cc MultiDB-conduitDialog.ui \
	MultiDB-factory.cc MultiDB-setup.cc MultiDBWidgetPrivate.cpp \
	Organizer-conduit.cc Organizer-factory.cc pilotOrganizerEntry.cc
libProjectsConduit_la_LDFLAGS = $(KDE_RPATH) -L$(top_srcdir)/kpilot/lib -L$(top_srcdir)/libkcal --no-undefined
libProjectsConduit_la_LIBADD = -lkpilot -lkcal 

