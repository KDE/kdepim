link_directories(${CMAKE_BINARY_DIR}/lib ${CMAKE_CURRENT_BINARY_DIR})

set(settings_SRC
	kpilotConfig.cc
)

kde3_add_kcfg_files(settings_SRC kpilotSettings.kcfgc)

set(shared_SRCS
	${settings_SRC}
	syncStack.cc
	hotSync.cc
	interactiveSync.cc
	logWidget.cc
	pilotComponent.cc
)

set(shared_STUBS
	loggerDCOP.h
)

kde3_add_dcop_skels(shared_SRCS ${shared_STUBS})
kde3_add_dcop_stubs(shared_SRCS ${shared_STUBS})

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

set(kcmpilot_STUBS
	pilotDaemonDCOP.h
)

# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
include_directories(
	${CMAKE_BINARY_DIR}/lib
	${CMAKE_SOURCE_DIR}/lib
	${CMAKE_CURRENT_BINARY_DIR}
)

kde3_add_ui_files(kcmpilot_SRCS ${kcmpilot_UIS})
kde3_add_kcfg_files(kcmpilot_SRCS ${kcmpilot_KCFGS})
kde3_add_dcop_skels(kcmpilot_SRCS ${kcmpilot_STUBS})
kde3_add_dcop_stubs(kcmpilot_SRCS ${kcmpilot_STUBS})
kde3_automoc(${kcmpilot_SRCS})

# Now add these generated files to the ADD_LIBRARY step
# If this is NOT done, then the ui_*.h files will not be generated
add_library(kcm_kpilot SHARED ${kcmpilot_SRCS})
set_target_properties(kcm_kpilot PROPERTIES LOCATION ${KDE3_PLUGIN_INSTALL_DIR}
	PREFIX ""
)

set(kpilotTest_SRCS
	${shared_SRCS}	
	main-test.cc
)

kde3_automoc(${kpilotTest_SRCS})
add_executable(kpilotTest ${kpilotTest_SRCS})
target_link_libraries(kpilotTest ${QT_LIBRARIES} kpilot)
kpilot_rpath(kpilotTest)

set(kpilotDaemon_SRCS
	${shared_SRCS}
	fileInstaller.cc
	internalEditorAction.cc
	logFile.cc
	pilotDaemon.cc
)

set(kpilot_STUBS
	kpilotDCOP.h
	pilotDaemonDCOP.h
)

kde3_add_dcop_skels(kpilotDaemon_SRCS ${kpilot_STUBS})
kde3_add_dcop_stubs(kpilotDaemon_SRCS ${kpilot_STUBS})
kde3_automoc(${kpilotDaemon_SRCS})
add_executable(kpilotDaemon ${kpilotDaemon_SRCS})
target_link_libraries(kpilotDaemon ${QT_LIBRARIES} kpilot)
kpilot_rpath(kpilotDaemon)

set(kpilot_SRCS
	${shared_SRCS}
	kpilot.cc
	dbviewerWidget.cc
	dbFlagsEditor.cc
	dbRecordEditor.cc
	dbAppInfoEditor.cc
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
kde3_add_dcop_skels(kpilot_SRCS ${kpilot_STUBS})
kde3_add_dcop_stubs(kpilot_SRCS ${kpilot_STUBS})
kde3_automoc(${kpilot_SRCS})
add_executable(kpilot_bin ${kpilot_SRCS})
target_link_libraries(kpilot_bin ${QT_LIBRARIES} kpilot kutils)
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

install(
	FILES kpilot.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share
)

install(
	FILES kpilotdaemon.desktop DESTINATION ${KDE3_XDG_APPS_DIR}
)

install(
	FILES kpilotconduit.desktop DESTINATION ${KDE3_SERVICETYPES_DIR}
)

install(
	FILES kpilot_config.desktop DESTINATION ${KDE3_SERVICES_DIR}
)

add_subdirectory(Icons)
