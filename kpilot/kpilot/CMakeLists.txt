# Don't forget to include output directory, otherwise
# the UI file won't be wrapped! 
include_directories(
	${CMAKE_CURRENT_BINARY_DIR}/../lib
	${CMAKE_CURRENT_SOURCE_DIR}/../lib
	${CMAKE_CURRENT_BINARY_DIR}
)
add_subdirectory(icons)

###
#
# Settings (KConfigXT) shared between various parts of KPilot
#
set(settings_SRC
	kpilotConfig.cc
)

kde4_add_kcfg_files(settings_SRC kpilotSettings.kcfgc)

###
#
# The KCM for KPilot, used by the config dialog and the embedding in Kontact
#
set(kcmpilot_SRCS
	${settings_SRC}
	config_dialog.cc
	config_dialog_probe.cc
	config_dialog_dbselection.cc
	config_pages.cc
)

set(kcmpilot_UIS
	config_dialog_dbselection_base.ui
	config_page_backup.ui
	config_page_device.ui
	config_page_startup.ui
	config_page_sync.ui
)

kde4_add_ui_files(kcmpilot_SRCS ${kcmpilot_UIS})

qt4_add_dbus_interface(kcmpilot_SRCS 
	org.kde.kpilot.daemon.xml
	kpilot_daemon_interface
)

kde4_add_plugin(kcm_kpilot ${kcmpilot_SRCS})

target_link_libraries(kcm_kpilot
	kpilot
	${KDE4_KDEUI_LIBS}
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
	main-test.cc
)
# Acts as a receiver for DBUS log messages
qt4_add_dbus_adaptor(kpilotTest_SRCS 
	org.kde.kpilot.logger.xml
	logWidget.h LogWidget 
	loggeradaptorgui LoggerAdaptorGUI
)

kde4_add_executable(kpilotTest ${kpilotTest_SRCS})

target_link_libraries(kpilotTest
	kpilot
	${QT_LIBRARIES}
	${KDE4_KIO_LIBS}
)

###
#
# The real KPilot daemon.
#
set(kpilotDaemon_SRCS
	${settings_SRC}
	hotSync.cc
	fileInstaller.cc
	logFile.cc
	pilotdaemontray.cc
	pilotDaemon.cc
)

qt4_add_dbus_adaptor(kpilotDaemon_SRCS
	org.kde.kpilot.logger.xml
	logFile.h LogFile
	loggeradaptorfile LoggerAdaptorFile
)

qt4_add_dbus_adaptor(kpilotDaemon_SRCS
	org.kde.kpilot.daemon.xml
	pilotDaemon.h PilotDaemon
)
qt4_add_dbus_interface(kpilotDaemon_SRCS
	org.kde.kpilot.logger.xml logfile_interface
)

qt4_add_dbus_interface(kpilotDaemon_SRCS
	org.kde.kpilot.kpilot.xml kpilot_interface
)

qt4_add_dbus_interface(kpilotDaemon_SRCS
	org.kde.kpilot.daemon.xml kpilot_daemon_interface
)

# todo: move to Oxygen icon
kde4_add_app_icon(kpilotDaemon_SRCS "icons/hi*-app-kpilotdaemon.png")

kde4_add_executable(kpilotDaemon ${kpilotDaemon_SRCS})

target_link_libraries(kpilotDaemon
	kpilot
	${QT_LIBRARIES}
	${KDE4_KIO_LIBS}
)

###
#
# The KPilot executable (viewer / editor / configuration).
#
set(kpilot_bin_SRCS
	${settings_SRC}
	logWidget.cc
	kpilot.cc
	fileInstaller.cc
	fileInstallWidget.cc
)

set(kpilot_UIS
	file_install_widget.ui
)

kde4_add_ui_files(kpilot_bin_SRCS ${kpilot_UIS})

# Talks to the daemon. Listens as itself.
qt4_add_dbus_adaptor(kpilot_bin_SRCS
	org.kde.kpilot.logger.xml
	logWidget.h LogWidget
	loggeradaptorgui LoggerAdaptorGUI
)

qt4_add_dbus_adaptor(kpilot_bin_SRCS
	org.kde.kpilot.kpilot.xml
	kpilot.h KPilotInstaller
)

qt4_add_dbus_interface(kpilot_bin_SRCS
	org.kde.kpilot.daemon.xml daemon_interface
)

# todo: move to Oxygen icon
kde4_add_app_icon(kpilot_bin_SRCS "icons/hi*-app-kpilot.png")

kde4_add_executable(kpilot_bin ${kpilot_bin_SRCS})

target_link_libraries(kpilot_bin 
	kpilot
	${QT_LIBRARIES}
	${KDE4_KUTILS_LIBS}
	${KDE4_KIO_LIBS}
)

set_target_properties(kpilot_bin PROPERTIES OUTPUT_NAME kpilot)

######################### INSTALL STUFF #######################################

install(TARGETS kcm_kpilot    DESTINATION ${PLUGIN_INSTALL_DIR})
install(TARGETS kpilot_bin    ${INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS kpilotDaemon  ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES   kpilotui.rc   DESTINATION ${DATA_INSTALL_DIR}/kpilot)
install(FILES
	kpilot.desktop
	kpilotdaemon.desktop
	DESTINATION ${XDG_APPS_INSTALL_DIR}
)
install(FILES   kpilotconduit.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
install(FILES   kpilot_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES   kpilot.upd            DESTINATION ${KCONF_UPDATE_INSTALL_DIR})
install(FILES   kpilot.kcfg           DESTINATION ${KCFG_INSTALL_DIR})
