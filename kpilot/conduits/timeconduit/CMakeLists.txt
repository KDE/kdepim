include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(conduit_time_SRCS
	time-conduit.cc
	time-factory.cc
	time-setup.cc
)

set(conduit_time_UIS
	time-setup_dialog.ui
)

set(conduit_time_KCFGS
	timeConduitSettings.kcfgc
)

kde4_add_kcfg_files(conduit_time_SRCS ${conduit_time_KCFGS})
kde4_add_ui_files(conduit_time_SRCS ${conduit_time_UIS})
kde4_add_plugin(conduit_time WITH_PREFIX ${conduit_time_SRCS})
target_link_libraries(conduit_time  ${KDE4_KIO_LIBS} kpilot)
install(TARGETS conduit_time DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES time_conduit.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES timeconduit.kcfg DESTINATION ${KCFG_INSTALL_DIR})
