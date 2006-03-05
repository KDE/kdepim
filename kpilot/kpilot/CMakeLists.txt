kde4_header()

add_subdirectory( Icons ) 

include_directories( ${CMAKE_SOURCE_DIR}/kpilot/lib ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### next target ###############

set(kpilotTest_SRCS 
   logWidget.cc 
   pilotComponent.cc 
   hotSync.cc 
   internalEditorAction.cc 
   interactiveSync.cc 
   syncStack.cc 
   main-test.cc )

kde4_automoc(${kpilotTest_SRCS})

kde4_add_dcop_skels(kpilotTest_SRCS loggerDCOP.h )

kde4_add_executable(kpilotTest ${kpilotTest_SRCS})

target_link_libraries(kpilotTest  ${QT_AND_KDECORE_LIBS} kpilot configuration )


########### next target ###############

set(kpilot_SRCS 
   kpilot.cc 
   pilotComponent.cc 
   logWidget.cc 
   memoWidget.cc 
   addressWidget.cc 
   addressEditor.cc 
   dbviewerWidget.cc 
   dbFlagsEditor.cc 
   dbRecordEditor.cc 
   dbAppInfoEditor.cc 
   datebookWidget.cc 
   todoWidget.cc 
   todoEditor.cc 
   fileInstaller.cc 
   fileInstallWidget.cc 
   listItems.cc )

kde4_automoc(${kpilot_SRCS})

kde4_add_ui_files(kpilot_SRCS dbFlagsEditor_base.ui todoEditor_base.ui )

kde4_add_dcop_skels(kpilot_SRCS kpilotDCOP.h loggerDCOP.h )

kde4_add_dcop_stubs(kpilot_SRCS pilotDaemonDCOP.h )

kde4_add_executable(kpilot ${kpilot_SRCS})

target_link_libraries(kpilot  ${QT_AND_KDECORE_LIBS} kutils kpilot configuration )

install_targets(/bin kpilot )


########### next target ###############

set(kpilotDaemon_SRCS 
   pilotDaemon.cc 
   logFile.cc 
   hotSync.cc 
   internalEditorAction.cc 
   interactiveSync.cc 
   syncStack.cc 
   fileInstaller.cc )

kde4_automoc(${kpilotDaemon_SRCS})

kde4_add_dcop_skels(kpilotDaemon_SRCS pilotDaemonDCOP.h loggerDCOP.h )

kde4_add_dcop_stubs(kpilotDaemon_SRCS kpilotDCOP.h loggerDCOP.h )

kde4_add_executable(kpilotDaemon ${kpilotDaemon_SRCS})

target_link_libraries(kpilotDaemon  ${QT_AND_KDECORE_LIBS} kpilot configuration )

install_targets(/bin kpilotDaemon )


########### next target ###############

set(kcm_kpilot_PART_SRCS 
   kpilotConfigDialog.cc 
   conduitConfigDialog.cc 
   kpilotProbeDialog.cc 
   kpilotConfigWizard.cc 
   dbSelectionDialog.cc )

kde4_automoc(${kcm_kpilot_PART_SRCS})

kde4_add_ui_files(kcm_kpilot_PART_SRCS 
   kpilotConfigDialog_device.ui 
   kpilotConfigDialog_sync.ui 
   kpilotConfigDialog_startup.ui 
   kpilotConfigDialog_viewers.ui 
   kpilotConfigDialog_backup.ui 
   kpilotConfigWizard_user.ui 
   kpilotConfigWizard_app.ui 
   dbSelection_base.ui )

kde4_add_dcop_stubs(kcm_kpilot_PART_SRCS pilotDaemonDCOP.h )

kde4_add_kcfg_files(kcm_kpilot_PART_SRCS kpilotConfigWizard_address.kcfgc kpilotConfigWizard_notes.kcfgc kpilotConfigWizard_vcal.kcfgc )

kde4_add_plugin(kcm_kpilot ${kcm_kpilot_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kpilot )

target_link_libraries(kcm_kpilot  ${QT_AND_KDECORE_LIBS} kpilot configuration )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kpilot )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kpilot.desktop  kpilotdaemon.desktop )
install_files( ${KCFG_INSTALL_DIR} FILES kpilot.kcfg )
install_files( ${DATA_INSTALL_DIR}/kpilot FILES kpilotui.rc )
install_files( ${SERVICES_INSTALL_DIR} FILES kpilot_config.desktop )
install_files( ${SERVICETYPES_INSTALL_DIR} FILES kpilotconduit.desktop )
install_files( ${DATA_INSTALL_DIR}/kconf_update FILES kpilot.upd )

kde4_footer()



#original Makefile.am contents follow:

#### Makefile.am for KPilot
####
#### This is a bit of a mess, but that's partly because we build two
#### executables and one library from the sources in this dir.
####
####
####
#
#SUBDIRS = Icons
#
#noinst_LTLIBRARIES = libconfiguration.la
#noinst_PROGRAMS = kpilotTest
#bin_PROGRAMS = kpilot kpilotDaemon
#kde_module_LTLIBRARIES = kcm_kpilot.la
#
#METASOURCES = AUTO
#
#INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
#
#libconfiguration_la_SOURCES = kpilotConfig.cc kpilotSettings.kcfgc
#libconfiguration_la_LDFLAGS = $(PISOCK_LDFLAGS) $(all_libraries)
#
#kcm_kpilot_la_SOURCES = pilotDaemonDCOP.stub  kpilotConfigDialog.cc \
#	kpilotConfigDialog_device.ui \
#	kpilotConfigDialog_sync.ui \
#	kpilotConfigDialog_startup.ui \
#	kpilotConfigDialog_viewers.ui \
#	kpilotConfigDialog_backup.ui \
#	conduitConfigDialog.cc \
#	kpilotProbeDialog.cc \
#	kpilotConfigWizard_address.kcfgc \
#	kpilotConfigWizard_notes.kcfgc \
#	kpilotConfigWizard_vcal.kcfgc \
#	kpilotConfigWizard_user.ui \
#	kpilotConfigWizard_app.ui \
#	kpilotConfigWizard.cc \
#	dbSelectionDialog.cc dbSelection_base.ui
#kcm_kpilot_la_LDFLAGS = $(PISOCK_LDFLAGS) $(all_libraries) -module -avoid-version -no-undefined
#kcm_kpilot_la_LIBADD = $(LIB_KFILE) $(PISOCK_LIB) \
#	../lib/libkpilot.la \
#	libconfiguration.la
#
#
#kpilot_SOURCES	= \
#	pilotDaemonDCOP.stub kpilotDCOP.skel loggerDCOP.skel \
#	kpilot.cc \
#	pilotComponent.cc logWidget.cc memoWidget.cc \
#	addressWidget.cc addressEditor.cc \
#	dbviewerWidget.cc dbFlagsEditor.cc dbFlagsEditor_base.ui \
#	dbRecordEditor.cc dbAppInfoEditor.cc \
#	datebookWidget.cc \
#	todoWidget.cc todoEditor.cc todoEditor_base.ui \
#	fileInstaller.cc fileInstallWidget.cc \
#	listItems.cc
#kpilot_COMPILE_FIRST = kpilotSettings.h
#
#
#kpilotDaemon_SOURCES = \
#	pilotDaemonDCOP.skel kpilotDCOP.stub loggerDCOP.stub loggerDCOP.skel \
#	pilotDaemon.cc logFile.cc \
#	hotSync.cc internalEditorAction.cc interactiveSync.cc syncStack.cc \
#	fileInstaller.cc
#
#kpilotTest_SOURCES = \
#	logWidget.cc pilotComponent.cc \
#	hotSync.cc internalEditorAction.cc interactiveSync.cc \
#	loggerDCOP.skel \
#	syncStack.cc \
#	main-test.cc
#
#
#kpilot_LDFLAGS = $(PISOCK_LDFLAGS) $(all_libraries) $(KDE_RPATH)
#kpilotDaemon_LDFLAGS = $(PISOCK_LDFLAGS) $(all_libraries) $(KDE_RPATH)
#kpilotTest_LDFLAGS = $(PISOCK_LDFLAGS) $(all_libraries) $(KDE_RPATH)
#
#
## the libraries to link against. Be aware of the order. First the libraries,
## that depend on the following ones.
#kpilot_LDADD		= $(LIB_KFILE) $(PISOCK_LIB) -lkutils \
#	../lib/libkpilot.la  libconfiguration.la
#kpilotDaemon_LDADD	= $(LIB_KFILE) $(PISOCK_LIB) \
#	../lib/libkpilot.la libconfiguration.la
#kpilotTest_LDADD	= $(LIB_KFILE) $(PISOCK_LIB) \
#	../lib/libkpilot.la libconfiguration.la
#
#xdg_apps_DATA = kpilot.desktop  kpilotdaemon.desktop
#kde_kcfg_DATA = kpilot.kcfg
#kde_services_DATA = kpilot_config.desktop
#
#servicetypedir = $(kde_servicetypesdir)
#servicetype_DATA = kpilotconduit.desktop
#
#rcdir = $(kde_datadir)/kpilot
#rc_DATA = kpilotui.rc
#
#update_DATA = kpilot.upd
#updatedir = $(kde_datadir)/kconf_update
#
#### Some additional dependencies:
#
#kpilotConfigDialog.lo : kpilotSettings.h
#
