bin_PROGRAMS = 	kpilot kpilotDaemon pilotListener
noinst_LIBRARIES = libconduit.a

METASOURCES = AUTO

#SUBDIRS = 
INCLUDES= $(all_includes)


# Which sources should be compiled for kpilot
kpilot_SOURCES	= kpilot.cc kpilotlink.cc fileInstallWidget.cc pilotMemo.cc memoWidget.cc pilotComponent.cc \
	kpilotOptions.cc messageDialog.cc gsetupDialog.cc \
	pilotSerialDatabase.cc pilotLocalDatabase.cc pilotRecord.cc pilotAddress.cc \
	addressWidget.cc strToken.cc addressEditor.cpp addressEditorData.cpp pilotDateEntry.cc \
	logFile.cc conduitSetup.cc pilotTodoEntry.cc

kpilotDaemon_SOURCES = pilotDaemon.cc kpilotlink.cc pilotSerialDatabase.cc pilotLocalDatabase.cc pilotRecord.cc \
	messageDialog.cc logFile.cc

pilotListener_SOURCES = pilotListener.cc

libconduit_a_SOURCES = baseConduit.cc conduitApp.cc \
	pilotRecord.cc pilotSerialDatabase.cc pilotLocalDatabase.cc \
	messageDialog.cc gsetupDialog.cc \
	kpilotlink.cc

kpilot_LDFLAGS = $(all_libraries)
kpilotDaemon_LDFLAGS = $(all_libraries)

kpilot_LDADD   = $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) -lqt -lX11 -lXext @LIBSOCKET@ -lpisock -lpicc
kpilotDaemon_LDADD = $(LIB_KDEUI) $(LIB_KDECORE) -lqt -lX11 -lXext @LIBSOCKET@ -lpisock

appsdir = $(kde_appsdir)/Utilities
apps_DATA = kpilot.desktop kpilotdaemon.desktop

messages:
	$(XGETTEXT) $(kpilot_SOURCES) -o $(podir)/kpilot.pot

