include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../lib
)

set(kpilot_conduit_time_SRCS
	time-conduit.cc
	time-factory.cc
	time-setup.cc
)

set(kpilot_conduit_time_UIS
	time-setup_dialog.ui
)

set(kpilot_conduit_time_KCFGS
	timeConduitSettings.kcfgc
)

kde4_add_kcfg_files(kpilot_conduit_time_SRCS ${kpilot_conduit_time_KCFGS})
kde4_add_ui_files(kpilot_conduit_time_SRCS ${kpilot_conduit_time_UIS})
kde4_add_plugin(kpilot_conduit_time ${kpilot_conduit_time_SRCS})
target_link_libraries(kpilot_conduit_time  ${KDE4_KIO_LIBS} kpilot)
install(TARGETS kpilot_conduit_time DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES time_conduit.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES timeconduit.kcfg DESTINATION ${KCFG_INSTALL_DIR})
