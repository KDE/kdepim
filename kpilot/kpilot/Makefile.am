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

bin_PROGRAMS = kpilot kpilotDaemon pilotListener
noinst_LIBRARIES = libconduit.a libpilotdb.a

# WABA: We can't build with enable-final because conduits/vcalconduit links
# with pilotDateEntry.o -- it's now libconduit.a and libpilotdb.a, so we
# *can* use --enable-final.
# KDE_OPTIONS = nofinal

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
	fileInstaller.cc fileInstallWidget.cc \
	memoWidget.cc \
	addressWidget.cc addressEditor.cc \
	options.cc

kpilotDaemon_SOURCES = pilotDaemon.cc \
	kpilotConfig.cc kpilotlink.cc \
	fileInstaller.cc \
	messageDialog.cc \
	pilotDaemonDCOP.skel \
	options.cc

pilotListener_SOURCES = pilotListener.cc

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

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
kpilot_LDADD		= -lpilotdb $(LIB_KFILE) $(LIBPISOCK)
kpilotDaemon_LDADD	= -lpilotdb $(LIB_KFILE) $(LIBPISOCK)

appsdir = $(kde_appsdir)/Utilities
servicetypedir = $(kde_servicetypesdir)
apps_DATA = kpilot.desktop kpilotdaemon.desktop
servicetype_DATA = kpilotconduit.desktop

kpiloticondir = $(kde_datadir)/kpilot/icons
kpiloticon_ICON = backup fastsync hotsync restore

KDE_ICON = kpilot

rcdir = $(kde_datadir)/kpilot
rc_DATA = kpilotui.rc
