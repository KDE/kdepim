include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(kpilot_conduit_sysinfo_SRCS
	sysinfo-setup.cc
	sysinfo-factory.cc
	sysinfo-conduit.cc
)

set(kpilot_conduit_sysinfo_UIS
	sysinfo-setup_dialog.ui
)

set(kpilot_conduit_sysinfo_KCFGS
	sysinfoSettings.kcfgc
)

kde4_add_kcfg_files(kpilot_conduit_sysinfo_SRCS ${kpilot_conduit_sysinfo_KCFGS})
kde4_add_ui3_files(kpilot_conduit_sysinfo_SRCS ${kpilot_conduit_sysinfo_UIS})

kde4_add_plugin(kpilot_conduit_sysinfo ${kpilot_conduit_sysinfo_SRCS})
target_link_libraries(kpilot_conduit_sysinfo  ${KDE4_KIO_LIBS} kpilot)

#install(TARGETS kpilot_conduit_sysinfo	DESTINATION ${PLUGIN_INSTALL_DIR})
#install(FILES sysinfo_conduit.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#install(FILES sysinfoconduit.kcfg DESTINATION ${KCFG_INSTALL_DIR})
#install(FILES Template.html Template.txt DESTINATION ${DATA_INSTALL_DIR}/kpilot/sysinfoconduit)

