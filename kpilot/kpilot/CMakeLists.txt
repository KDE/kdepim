link_directories(${CMAKE_BINARY_DIR}/lib ${CMAKE_CURRENT_BINARY_DIR})

# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
include_directories(
	${CMAKE_BINARY_DIR}/lib
	${CMAKE_SOURCE_DIR}/lib
	${CMAKE_CURRENT_BINARY_DIR}
)


###
#
# Settings (KConfigXT) shared between various parts of KPilot
#
set(settings_SRC
	kpilotConfig.cc
)

kde3_add_kcfg_files(settings_SRC kpilotSettings.kcfgc)

###
#
# The KCM for KPilot, used by the config dialog and the embedding in Kontact
#
set(kcmpilot_SRCS
	${settings_SRC}
	kpilotConfigWizard.cc
	dbSelectionDialog.cc
	kpilotConfigDialog.cc
	conduitConfigDialog.cc
	kpilotProbeDialog.cc
)

set(kcmpilot_KCFGS
	kpilotConfigWizard_address.kcfgc
	kpilotConfigWizard_notes.kcfgc
	kpilotConfigWizard_vcal.kcfgc
)

set(kcmpilot_UIS
	kpilotConfigDialog_device.ui
	kpilotConfigDialog_sync.ui
	kpilotConfigDialog_startup.ui
	kpilotConfigDialog_viewers.ui
	kpilotConfigDialog_backup.ui
	kpilotConfigWizard_user.ui
	kpilotConfigWizard_app.ui
	dbSelection_base.ui
)

kde3_add_ui_files(kcmpilot_SRCS ${kcmpilot_UIS})
kde3_add_kcfg_files(kcmpilot_SRCS ${kcmpilot_KCFGS})
kde3_add_dcop_stubs(kcmpilot_SRCS pilotDaemonDCOP.h)
kde3_automoc(${kcmpilot_SRCS})

# Now add these generated files to the ADD_LIBRARY step
# If this is NOT done, then the ui_*.h files will not be generated
add_library(kcm_kpilot SHARED ${kcmpilot_SRCS})
set_target_properties(kcm_kpilot PROPERTIES
	LOCATION ${KDE3_PLUGIN_INSTALL_DIR}
	PREFIX ""
)

###
#
# Most simple daemon-like application: kpilotTest, which
# runs whatever the command-line tells it to.
#
set(kpilotTest_SRCS
	${settings_SRC}
	hotSync.cc
	logWidget.cc
	pilotComponent.cc
	main-test.cc
)
# Acts as a receiver for DCOP log messages
kde3_add_dcop_skels(kpilotTest_SRCS loggerDCOP.h)

kde3_automoc(${kpilotTest_SRCS})
add_executable(kpilotTest ${kpilotTest_SRCS})
target_link_libraries(kpilotTest ${QT_LIBRARIES} kpilot kdeui kio)
kpilot_rpath(kpilotTest)

###
#
# The real KPilot daemon.
#
set(kpilotDaemon_SRCS
	${settings_SRC}
	hotSync.cc
	fileInstaller.cc
	internalEditorAction.cc
	logFile.cc
	pilotDaemon.cc
)
# Both sends and receives DCOP log messages. Talks to KPilot.
kde3_add_dcop_skels(kpilotDaemon_SRCS loggerDCOP.h pilotDaemonDCOP.h)
kde3_add_dcop_stubs(kpilotDaemon_SRCS loggerDCOP.h kpilotDCOP.h)
kde3_automoc(${kpilotDaemon_SRCS})
add_executable(kpilotDaemon ${kpilotDaemon_SRCS})
target_link_libraries(kpilotDaemon ${QT_LIBRARIES} kpilot kdeui kio)
kpilot_rpath(kpilotDaemon)

###
#
# The KPilot executable (viewer / editor / configuration).
#
set(kpilot_SRCS
	${settings_SRC}
	logWidget.cc
	kpilot.cc
	dbviewerWidget.cc
	dbFlagsEditor.cc
	dbRecordEditor.cc
	dbAppInfoEditor.cc
	pilotComponent.cc
	memoWidget.cc
	addressWidget.cc
	addressEditor.cc
	datebookWidget.cc
	todoWidget.cc
	todoEditor.cc
	fileInstaller.cc
	fileInstallWidget.cc
	listItems.cc
)

set(kpilot_UIS
	dbFlagsEditor_base.ui
	todoEditor_base.ui
)

kde3_add_ui_files(kpilot_SRCS ${kpilot_UIS})
# Talks to the daemon. Listens as itself.
kde3_add_dcop_skels(kpilot_SRCS loggerDCOP.h kpilotDCOP.h)
kde3_add_dcop_stubs(kpilot_SRCS pilotDaemonDCOP.h)
kde3_automoc(${kpilot_SRCS})
add_executable(kpilot_bin ${kpilot_SRCS})
target_link_libraries(kpilot_bin ${QT_LIBRARIES} kpilot kutils kdeui kio)
kpilot_rpath(kpilot_bin)
set_target_properties(kpilot_bin PROPERTIES
	OUTPUT_NAME kpilot
)

######################### INSTALL STUFF #######################################

kde3_install_libtool_file(kcm_kpilot)

install(
	TARGETS kcm_kpilot kpilot_bin kpilotDaemon
	LIBRARY DESTINATION ${KDE3_PLUGIN_INSTALL_DIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
	RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

install( FILES kpilotui.rc DESTINATION ${CMAKE_INSTALL_PREFIX}/share/apps/kpilot)

install(
	FILES kpilot.desktop kpilotdaemon.desktop DESTINATION ${KDE3_XDG_APPS_DIR}
)

install(
	FILES kpilotconduit.desktop DESTINATION ${KDE3_SERVICETYPES_DIR}
)

install(
	FILES kpilot_config.desktop DESTINATION ${KDE3_SERVICES_DIR}
)

install ( 
	FILES kpilot.upd 
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/apps/kconf_update
)

install(
	FILES kpilot.kcfg DESTINATION ${KDE3_KCFG_DIR}
)

add_subdirectory(Icons)
