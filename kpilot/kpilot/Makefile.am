bin_PROGRAMS = kpilot kpilotDaemon pilotListener
noinst_LIBRARIES = libconduit.a

# WABA: We can't build with enable-final because conduits/vclaconduit links
# with pilotDateEntry.o
KDE_OPTIONS = nofinal

METASOURCES = AUTO

INCLUDES= $(all_includes) -I$(LIBPISOCK_PATH)

#SUBDIRS = doc

kpilot_SOURCES	= kpilot.cc kpilotlink.cc fileInstallWidget.cc pilotMemo.cc memoWidget.cc pilotComponent.cc \
	kpilotOptions.cc messageDialog.cc gsetupDialog.cc \
	pilotSerialDatabase.cc pilotLocalDatabase.cc pilotRecord.cc pilotAddress.cc \
	listItems.cc \
	addressWidget.cc addressEditor.cc \
	strToken.cc pilotDateEntry.cc \
	conduitSetup.cc listCat.cc pilotTodoEntry.cc \
	options.cc 

kpilotDaemon_SOURCES = pilotDaemon.cc kpilotlink.cc pilotSerialDatabase.cc pilotLocalDatabase.cc pilotRecord.cc \
	messageDialog.cc \
	pilotDaemonDCOP.skel \
	options.cc

pilotListener_SOURCES = pilotListener.cc

libconduit_a_SOURCES = baseConduit.cc conduitApp.cc \
	pilotRecord.cc pilotSerialDatabase.cc pilotLocalDatabase.cc \
	messageDialog.cc gsetupDialog.cc \
	kpilotlink.cc options.cc

kpilot_LDFLAGS = @all_libraries@ -L/usr/local/lib $(KDE_RPATH)
kpilotDaemon_LDFLAGS = @all_libraries@ -L/usr/local/lib $(KDE_RPATH)

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
kpilot_LDADD   = $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) -lqt -lX11 -lXext @LIBSOCKET@ $(LIBPISOCK)
# -lpicc
kpilotDaemon_LDADD = $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) -lqt -lX11 -lXext @LIBSOCKET@ $(LIBPISOCK)

appsdir = $(kde_appsdir)/Utilities
apps_DATA = kpilot.desktop kpilotdaemon.desktop
servicetype_DATA = kpilotconduit.desktop
servicetypedir = $(kde_servicetypesdir)
