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
### noinst_LTLIBRARIES = libkhexedit_shared.la
bin_PROGRAMS = kpilot kpilotDaemon

METASOURCES = AUTO

INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)

###
### On some platforms, such as stock RH 7.1, the KDE libraries are built without
### kdDebug() support. You may need to add -DDEBUG_CERR to the CXXFLAGS, if
### you want any debugging at all.
###
### KDE_CXXFLAGS=-DNDEBUG -UDEBUG
### KDE_CXXFLAGS=-DDEBUG

###
### Non-automake variables
###

### libkhexedit_shared_la_SOURCES = hexbuffer.cc hexclipboard.cc hexdrag.cc hexerror.cc hexviewwidget.cc hexprinter.cc conversion.cc hexviewwidget.cc


kpilot_SOURCES	= \
	pilotDaemonDCOP.stub kpilotDCOP.skel logWidgetDCOP.skel \
	kpilot.cc kpilotConfig.cc \
	kpilotConfigDialog.cc kpilotConfigDialog_base.ui \
	conduitConfigDialog.cc \
	dbSelectionDialog.cc dbSelection_base.ui \
	pilotComponent.cc logWidget.cc memoWidget.cc \
	addressWidget.cc addressEditor.cc \
	dbviewerWidget.cc dbFlagsEditor.cc dbFlagsEditor_base.ui\
	datebookWidget.cc \
	todoWidget.cc todoEditor.cc \
	fileInstaller.cc fileInstallWidget.cc \
	listItems.cc \
	kroupware.cc


kpilotDaemon_SOURCES = \
	pilotDaemonDCOP.skel kpilotDCOP.stub logWidgetDCOP.stub \
	pilotDaemon.cc kpilotConfig.cc \
	hotSync.cc internalEditorAction.cc interactiveSync.cc syncStack.cc \
	fileInstaller.cc kroupware.cc

kpilotTest_SOURCES = kpilotConfig.cc \
	logWidget.cc pilotComponent.cc \
	hotSync.cc internalEditorAction.cc interactiveSync.cc \
	logWidgetDCOP.skel \
	syncStack.cc \
	main-test.cc

kpilotConfig_SOURCES = kpilotConfig.cc \
	kpilotConfigDialog.cc kpilotConfigDialog_base.ui \
	dbSelectionDialog.cc dbSelection_base.ui \
	conduitConfigDialog.cc \
	listCat.cc main-config.cc


## libkhexedit_shared_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kpilot_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kpilotDaemon_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kpilotTest_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kpilotConfig_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
### libkhexedit_shared_la_LIBADD = -lkdeprint
kpilot_LDADD		= $(LIB_KFILE) $(PISOCK_LIB) ../lib/libkpilot.la
### kpilot_LDADD		= $(LIB_KFILE) -lkdeprint $(PISOCK_LIB) ../lib/libkpilot.la libkhexedit_shared.la
kpilotDaemon_LDADD	= $(LIB_KFILE) $(PISOCK_LIB) ../lib/libkpilot.la
kpilotTest_LDADD	= $(LIB_KFILE) $(PISOCK_LIB) ../lib/libkpilot.la
kpilotConfig_LDADD      = $(LIB_KFILE) $(PISOCK_LIB) ../lib/libkpilot.la

appsdir = $(kde_appsdir)/Utilities
servicetypedir = $(kde_servicetypesdir)
apps_DATA = kpilot.desktop  kpilotdaemon.desktop
servicetype_DATA = kpilotconduit.desktop

rcdir = $(kde_datadir)/kpilot
rc_DATA = kpilotui.rc
