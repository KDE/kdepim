### Makefile.am for kpilot/lib
###
### $Id$
###

METASOURCES = AUTO

INCLUDES = $(all_includes) 

### If you must get debugging output on a platform where
### the libs are built without debugging support, define
### DEBUG_CERR. Define DEBUG to get debugging support anywhere.
###
### KDE_CXXFLAGS=-DDEBUG -DDEBUG_CERR
KDE_CXXFLAGS=-DDEBUG -DDEBUG_CERR

lib_LTLIBRARIES = libkpilot.la

libkpilot_la_SOURCES = options.cc uiDialog.cc plugin.cc syncAction.cc \
	kpilotlink.cc \
	pilotRecord.cc pilotDatabase.cc \
	pilotLocalDatabase.cc pilotSerialDatabase.cc \
	pilotMemo.cc pilotAddress.cc \
	pilotDateEntry.cc pilotTodoEntry.cc

libkpilot_la_LIBADD = $(KDE_EXTRA_RPATH) $(LIBPISOCK)

oldincludedir = $(includedir)/kpilot
oldinclude_HEADERS = kpilotlink.h uiDialog.h plugin.h syncAction.h \
	pilotRecord.h pilotDatabase.h \
	pilotLocalDatabase.h pilotSerialDatabase.h

