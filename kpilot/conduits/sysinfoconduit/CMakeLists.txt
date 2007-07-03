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

kde4_add_kcfg_files(conduit_sysinfo_SRCS ${conduit_sysinfo_KCFGS})
kde4_add_ui3_files(conduit_sysinfo_SRCS ${conduit_sysinfo_UIS})
kde4_automoc(${conduit_sysinfo_SRCS})

kde4_add_plugin(conduit_sysinfo WITH_PREFIX ${conduit_sysinfo_SRCS})
target_link_libraries(conduit_sysinfo  ${KDE4_KIO_LIBS} kpilot)

install(TARGETS conduit_sysinfo	DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES sysinfo_conduit.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES sysinfoconduit.kcfg DESTINATION ${KCFG_INSTALL_DIR})

install(FILES Template.html Template.txt DESTINATION ${DATA_INSTALL_DIR}/kpilot/sysinfoconduit)

