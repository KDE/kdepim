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
noinst_LIBRARIES = libconduit.a libpilotdb.a

METASOURCES = AUTO

INCLUDES= $(all_includes) -I$(LIBPISOCK_PATH)

###
### Non-automake variables
###

kpilot_SOURCES	= kpilot.cc \
	kpilotConfig.cc \
	messageDialog.cc gsetupDialog.cc \
	kpilotOptions.cc \
	pilotDaemonDCOP.stub \
	listItems.cc kpilotDCOP.skel \
	strToken.cc \
	conduitSetup.cc listCat.cc \
	pilotComponent.cc \
	logWidget.cc fileInstaller.cc fileInstallWidget.cc \
	memoWidget.cc \
	addressWidget.cc addressEditor.cc \
	options.cc

kpilotDaemon_SOURCES = pilotDaemon.cc \
	kpilotConfig.cc kpilotlink.cc hotSync.cc \
	fileInstaller.cc \
	messageDialog.cc \
	pilotDaemonDCOP.skel \
	kpilotDCOP.stub \
	options.cc

kpilotTest_SOURCES = options.cc kpilotConfig.cc kpilotlink.cc \
	logWidget.cc pilotComponent.cc \
	main-test.cc

kpilotConfig_SOURCES = options.cc kpilotConfig.cc \
	gsetupDialog.cc listCat.cc \
	conduitSetup.cc kpilotOptions.cc \
	main-config.cc
	

libconduit_a_SOURCES = baseConduit.cc conduitApp.cc \
	messageDialog.cc gsetupDialog.cc \
	kpilotConfig.cc kpilotlink.cc \
	options.cc

libpilotdb_a_SOURCES = \
	pilotRecord.cc \
	pilotTodoEntry.cc \
	pilotDateEntry.cc \
	pilotMemo.cc \
	pilotAddress.cc \
	pilotConduitDatabase.cc pilotSerialDatabase.cc pilotLocalDatabase.cc


kpilot_LDFLAGS = $(all_libraries) -L/usr/local/lib -L. $(KDE_RPATH)
kpilotDaemon_LDFLAGS = $(all_libraries) -L/usr/local/lib -L. $(KDE_RPATH)
kpilotTest_LDFLAGS = $(all_libraries) -L/usr/local/lib -L. $(KDE_RPATH)
kpilotConfig_LDFLAGS = $(all_libraries) -L/usr/local/lib -L. $(KDE_RPATH)

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
kpilot_LDADD		= -lpilotdb $(LIB_KFILE) $(LIBPISOCK)
kpilotDaemon_LDADD	= -lpilotdb $(LIB_KFILE) $(LIBPISOCK)
kpilotTest_LDADD	= $(LIB_KDEUI) $(LIBPISOCK)
kpilotConfig_LDADD      = $(LIB_KFILE)

appsdir = $(kde_appsdir)/Utilities
servicetypedir = $(kde_servicetypesdir)
apps_DATA = kpilot.desktop kpilotdaemon.desktop
servicetype_DATA = kpilotconduit.desktop

kpiloticondir = $(kde_datadir)/kpilot/icons
kpiloticon_ICON = backup fastsync hotsync restore

KDE_ICON = kpilot

rcdir = $(kde_datadir)/kpilot
rc_DATA = kpilotui.rc
