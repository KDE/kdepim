### Makefile.am for KPilot
###
### This is a bit of a mess, but that's partly because we build two
### executables and one library from the sources in this dir.
###
###
### $Id$
###

SUBDIRS = Icons

### Fairly large changes to allow enable final and make life easier for
### conduits (and others) -- now build all the database types and known
### database record types into a library.

noinst_PROGRAMS = kpilotTest kpilotConfig
bin_PROGRAMS = kpilot kpilotDaemon

METASOURCES = AUTO

INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)

###
### On some platforms, such as stock RH 7.1, the KDE libraries are built without
### kdDebug() support. You may need to add -DDEBUG_CERR to the CXXFLAGS, if
### you want any debugging at all.
###
### KDE_CXXFLAGS=-DNDEBUG -UDEBUG

###
### Non-automake variables
###

kpilot_SOURCES	= \
	pilotDaemonDCOP.stub kpilotDCOP.skel logWidgetDCOP.skel \
	kpilot.cc kpilotConfig.cc \
	kpilotConfigDialog.cc kpilotConfigDialog_base.ui \
	conduitConfigDialog.cc conduitConfigDialog_base.ui \
	pilotComponent.cc logWidget.cc memoWidget.cc addressWidget.cc \
	addressEditor.cc \
	fileInstaller.cc fileInstallWidget.cc \
	listItems.cc \
	kroupware.cc


kpilotDaemon_SOURCES = \
	pilotDaemonDCOP.skel kpilotDCOP.stub logWidgetDCOP.stub \
	pilotDaemon.cc kpilotConfig.cc \
	hotSync.cc interactiveSync.cc syncStack.cc \
	fileInstaller.cc kroupware.cc

kpilotTest_SOURCES = kpilotConfig.cc \
	logWidget.cc pilotComponent.cc \
	hotSync.cc interactiveSync.cc \
	logWidgetDCOP.skel \
	syncStack.cc \
	main-test.cc

kpilotConfig_SOURCES = kpilotConfig.cc \
	kpilotConfigDialog.cc kpilotConfigDialog_base.ui \
	conduitConfigDialog_base.ui conduitConfigDialog.cc \
	listCat.cc main-config.cc


kpilot_LDFLAGS = $(all_libraries) $(KDE_RPATH) -L../lib
kpilotDaemon_LDFLAGS = $(all_libraries) $(KDE_RPATH) -L../lib
kpilotTest_LDFLAGS = $(all_libraries) $(KDE_RPATH) -L../lib
kpilotConfig_LDFLAGS = $(all_libraries) $(KDE_RPATH) -L../lib

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
kpilot_LDADD		= $(LIB_KFILE) $(PISOCK_LIB) ../lib/libkpilot.la
kpilotDaemon_LDADD	= $(LIB_KFILE) $(PISOCK_LIB) ../lib/libkpilot.la
kpilotTest_LDADD	= $(LIB_KFILE) $(PISOCK_LIB) ../lib/libkpilot.la
kpilotConfig_LDADD      = $(LIB_KFILE) $(PISOCK_LIB) ../lib/libkpilot.la

appsdir = $(kde_appsdir)/Utilities
servicetypedir = $(kde_servicetypesdir)
apps_DATA = kpilot.desktop  kpilotdaemon.desktop
servicetype_DATA = kpilotconduit.desktop

rcdir = $(kde_datadir)/kpilot
rc_DATA = kpilotui.rc
