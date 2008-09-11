include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../lib
)

set(kpilot_conduit_popmail_SRCS
	popmail-factory.cc
	popmail-conduit.cc
	setupDialog.cc
)

set(kpilot_conduit_popmail_UIS
	setup-dialog.ui
)

set(kpilot_conduit_popmail_KCFGS
	popmailSettings.kcfgc
)

kde4_add_kcfg_files(kpilot_conduit_popmail_SRCS ${kpilot_conduit_popmail_KCFGS})
kde4_add_ui_files(kpilot_conduit_popmail_SRCS ${kpilot_conduit_popmail_UIS})

kde4_add_plugin(kpilot_conduit_popmail ${kpilot_conduit_popmail_SRCS})
target_link_libraries(kpilot_conduit_popmail  ${KDE4_KIO_LIBS} kpilot)

#install(TARGETS kpilot_conduit_popmail DESTINATION ${PLUGIN_INSTALL_DIR})
#install(FILES popmail-conduit.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#install(FILES popmail.kcfg DESTINATION ${KCFG_INSTALL_DIR})
