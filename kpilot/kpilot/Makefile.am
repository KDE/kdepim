### Makefile.am for KPilot
###
### This is a bit of a mess, but that's partly because we build two
### executables and one library from the sources in this dir.
###
###
### $Id$
###

SUBDIRS = Icons

noinst_LTLIBRARIES = libconfiguration.la
noinst_PROGRAMS = kpilotTest
bin_PROGRAMS = kpilot kpilotDaemon
kde_module_LTLIBRARIES = kcm_kpilot.la

METASOURCES = AUTO

INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib -I$(top_builddir)/libkdepim $(all_includes)

libconfiguration_la_SOURCES = kpilotConfig.cc kpilotSettings.kcfgc
libconfiguration_la_LDFLAGS = $(all_libraries)

kcm_kpilot_la_SOURCES = pilotDaemonDCOP.stub  kpilotConfigDialog.cc \
	kpilotConfigDialog_device.ui \
	kpilotConfigDialog_sync.ui \
	kpilotConfigDialog_startup.ui \
	kpilotConfigDialog_viewers.ui \
	kpilotConfigDialog_backup.ui \
	conduitConfigDialog.cc \
	kpilotProbeDialog.cc \
	kpilotConfigWizard_address.kcfgc \
	kpilotConfigWizard_notes.kcfgc \
	kpilotConfigWizard_vcal.kcfgc \
	kpilotConfigWizard_user.ui \
	kpilotConfigWizard_app.ui \
	kpilotConfigWizard.cc \
	dbSelectionDialog.cc dbSelection_base.ui
kcm_kpilot_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_kpilot_la_LIBADD = $(LIB_KFILE) $(PISOCK_LIB) \
	../lib/libkpilot.la \
	libconfiguration.la


kpilot_SOURCES	= \
	pilotDaemonDCOP.stub kpilotDCOP.skel loggerDCOP.skel \
	kpilot.cc \
	pilotComponent.cc logWidget.cc memoWidget.cc \
	addressWidget.cc addressEditor.cc \
	dbviewerWidget.cc dbFlagsEditor.cc dbFlagsEditor_base.ui \
	dbRecordEditor.cc dbAppInfoEditor.cc \
	datebookWidget.cc \
	todoWidget.cc todoEditor.cc todoEditor_base.ui \
	fileInstaller.cc fileInstallWidget.cc \
	listItems.cc
kpilot_COMPILE_FIRST = kpilotSettings.h


kpilotDaemon_SOURCES = \
	pilotDaemonDCOP.skel kpilotDCOP.stub loggerDCOP.stub loggerDCOP.skel \
	pilotDaemon.cc logFile.cc \
	hotSync.cc internalEditorAction.cc interactiveSync.cc syncStack.cc \
	fileInstaller.cc

kpilotTest_SOURCES = \
	logWidget.cc pilotComponent.cc \
	hotSync.cc internalEditorAction.cc interactiveSync.cc \
	loggerDCOP.skel \
	syncStack.cc \
	main-test.cc


kpilot_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kpilotDaemon_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kpilotTest_LDFLAGS = $(all_libraries) $(KDE_RPATH)


# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
kpilot_LDADD		= $(LIB_KFILE) $(PISOCK_LIB) -lkutils \
	../lib/libkpilot.la  libconfiguration.la
kpilotDaemon_LDADD	= $(LIB_KFILE) $(PISOCK_LIB) \
	../lib/libkpilot.la libconfiguration.la
kpilotTest_LDADD	= $(LIB_KFILE) $(PISOCK_LIB) \
	../lib/libkpilot.la libconfiguration.la

xdg_apps_DATA = kpilot.desktop  kpilotdaemon.desktop
kde_kcfg_DATA = kpilot.kcfg
kde_services_DATA = kpilot_config.desktop

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kpilotconduit.desktop

rcdir = $(kde_datadir)/kpilot
rc_DATA = kpilotui.rc

update_DATA = kpilot.upd
updatedir = $(kde_datadir)/kconf_update

### Some additional dependencies:

kpilotConfigDialog.lo : kpilotSettings.h

