include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(conduit_popmail_SRCS
	popmail-factory.cc
	popmail-conduit.cc
	setupDialog.cc
)

set(conduit_popmail_UIS
	setup-dialog.ui
)

set(conduit_popmail_KCFGS
	popmailSettings.kcfgc
)

kde4_add_kcfg_files(conduit_popmail_SRCS ${conduit_popmail_KCFGS})
kde4_add_ui_files(conduit_popmail_SRCS ${conduit_popmail_UIS})
kde4_automoc(${conduit_popmail_SRCS})

kde4_add_plugin(conduit_popmail WITH_PREFIX ${conduit_popmail_SRCS})
target_link_libraries(conduit_popmail ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} kpilot)
install(TARGETS conduit_popmail DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES popmail-conduit.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES popmail.kcfg DESTINATION ${KCFG_INSTALL_DIR})
