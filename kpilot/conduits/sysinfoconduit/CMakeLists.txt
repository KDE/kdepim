include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(conduit_sysinfo_SRCS
	sysinfo-setup.cc
	sysinfo-factory.cc
	sysinfo-conduit.cc
)

set(conduit_sysinfo_UIS
	sysinfo-setup_dialog.ui
)

set(conduit_sysinfo_KCFGS
	sysinfoSettings.kcfgc
)

kde3_add_kcfg_files(conduit_sysinfo_SRCS ${conduit_sysinfo_KCFGS})
kde3_add_ui_files(conduit_sysinfo_SRCS ${conduit_sysinfo_UIS})
kde3_automoc(${conduit_sysinfo_SRCS})
add_library(conduit_sysinfo SHARED ${conduit_sysinfo_SRCS})

set_target_properties(
	conduit_sysinfo PROPERTIES LOCATION ${KDE3_PLUGIN_INSTALL_DIR}
	INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib
	PREFIX ""
)

kde3_install_libtool_file(conduit_sysinfo)

install(
	TARGETS conduit_sysinfo
	LIBRARY DESTINATION ${KDE3_PLUGIN_INSTALL_DIR}
)

install(
	FILES sysinfo_conduit.desktop DESTINATION ${KDE3_SERVICES_DIR}
)

install(
	FILES sysinfoSettings.kcfgc sysinfoconduit.kcfg
	DESTINATION ${KDE3_KCFG_DIR}
)

install(
	FILES Template.html Template.txt 
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/apps/kpilot/sysinfoconduit
)

