### Makefile.am for kpilot/lib
###
### $Id$
###

METASOURCES = AUTO

INCLUDES = $(PISOCK_INCLUDE) $(all_includes)

### If you must get debugging output on a platform where
### the libs are built without debugging support, define
### DEBUG_CERR. Define DEBUG to get debugging support anywhere.
###
### KDE_CXXFLAGS=-DDEBUG -DDEBUG_CERR
##KDE_CXXFLAGS=-DNDEBUG -UDEBUG
##KDE_CXXFLAGS=-DDEBUG

lib_LTLIBRARIES = libkpilot.la

libkpilot_la_SOURCES = kpilotlibSettings.kcfgc options.cc uiDialog.cc plugin.cc syncAction.cc \
	kpilotlink.cc recordConduit.cc \
	pilotRecord.cc pilotDatabase.cc \
	pilotLocalDatabase.cc pilotSerialDatabase.cc \
	pilotMemo.cc pilotAddress.cc \
	pilotDateEntry.cc pilotTodoEntry.cc

libkpilot_la_LDFLAGS = $(all_libraries) $(KDE_EXTRA_RPATH) $(KDE_RPATH)
libkpilot_la_LIBADD = $(PISOCK_LIB)

kpilotincludedir = $(includedir)/kpilot
kpilotinclude_HEADERS = kpilotlink.h uiDialog.h plugin.h syncAction.h recordConduit.h \
	pilotRecord.h pilotDatabase.h \
	pilotLocalDatabase.h pilotSerialDatabase.h

kde_kcfg_DATA = kpilotlib.kcfg

include ../../admin/Doxyfile.am
