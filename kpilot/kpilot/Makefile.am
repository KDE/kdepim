### Makefile.am for KPilot
###
### This is a bit of a mess, but that's partly because we build two
### executables and one library from the sources in this dir.
###
###
### $Id$
###

### Fairly large changes to allow enable final and make life easier for
### conduits (and others) -- now build all the database types and known
### database record types into a library.

noinst_PROGRAMS = kpilotTest kpilotConfig
bin_PROGRAMS = kpilot kpilotDaemon

METASOURCES = AUTO

INCLUDES= $(all_includes) -I$(LIBPISOCK_PATH)

###
### On some platforms, such as stock RH 7.1, the KDE libraries are built without
### kdDebug() support. You may need to add -DDEBUG_CERR to the CXXFLAGS.
###
CXXFLAGS+=-DDEBUG -DDEBUG_CERR

###
### Non-automake variables
###

kpilot_SOURCES	= options.cc \
	pilotDaemonDCOP.stub kpilotDCOP.skel \
	kpilot.cc kpilotConfig.cc \
	uiDialog.cc kpilotConfigDialog.cc kpilotConfigDialog_base.ui \
	pilotComponent.cc logWidget.cc memoWidget.cc addressWidget.cc \
	addressEditor.cc \
	fileInstaller.cc fileInstallWidget.cc \
	conduitSetup.cc listCat.cc listItems.cc \
	pilotRecord.cc pilotLocalDatabase.cc pilotAddress.cc pilotMemo.cc


kpilotDaemon_SOURCES = options.cc \
	pilotDaemonDCOP.skel kpilotDCOP.stub \
	pilotDaemon.cc kpilotConfig.cc \
	kpilotlink.cc hotSync.cc interactiveSync.cc \
	fileInstaller.cc

kpilotTest_SOURCES = options.cc kpilotConfig.cc kpilotlink.cc \
	logWidget.cc pilotComponent.cc \
	hotSync.cc interactiveSync.cc main-test.cc

kpilotConfig_SOURCES = options.cc kpilotConfig.cc \
	uiDialog.cc kpilotConfigDialog.cc kpilotConfigDialog_base.ui \
	main-config.cc


kpilot_LDFLAGS = $(all_libraries) -L/usr/local/lib -L. $(KDE_RPATH)
kpilotDaemon_LDFLAGS = $(all_libraries) -L/usr/local/lib -L. $(KDE_RPATH)
kpilotTest_LDFLAGS = $(all_libraries) -L/usr/local/lib -L. $(KDE_RPATH)
kpilotConfig_LDFLAGS = $(all_libraries) -L/usr/local/lib -L. $(KDE_RPATH)

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
kpilot_LDADD		= $(LIB_KFILE) $(LIBPISOCK)
kpilotDaemon_LDADD	= $(LIB_KFILE) $(LIBPISOCK)
kpilotTest_LDADD	= $(LIB_KDEUI) $(LIBPISOCK)
kpilotConfig_LDADD      = $(LIB_KFILE)

kpilotConfig_CXXADD = -DDEBUG -DDEBUG_CERR

appsdir = $(kde_appsdir)/Utilities
servicetypedir = $(kde_servicetypesdir)
apps_DATA = kpilot.desktop kpilotdaemon.desktop
servicetype_DATA = kpilotconduit.desktop

kpiloticondir = $(kde_datadir)/kpilot/icons
kpiloticon_ICON = backup fastsync hotsync restore

KDE_ICON = kpilot

rcdir = $(kde_datadir)/kpilot
rc_DATA = kpilotui.rc
