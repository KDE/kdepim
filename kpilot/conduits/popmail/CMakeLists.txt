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

kde3_add_kcfg_files(conduit_popmail_SRCS ${conduit_popmail_KCFGS})
kde3_add_ui_files(conduit_popmail_SRCS ${conduit_popmail_UIS})
kde3_automoc(${conduit_popmail_SRCS})
add_library(conduit_popmail SHARED ${conduit_popmail_SRCS})

set_target_properties(
	conduit_popmail PROPERTIES LOCATION ${KDE3_PLUGIN_INSTALL_DIR}
	INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib
	PREFIX ""
)

kde3_install_libtool_file(conduit_popmail)

install(
	TARGETS conduit_popmail
	LIBRARY DESTINATION ${KDE3_PLUGIN_INSTALL_DIR}
)

install(
	FILES popmail-conduit.desktop DESTINATION ${KDE3_SERVICES_DIR}
)

install(
	FILES popmail.kcfg DESTINATION ${KDE3_KCFG_DIR}
)
