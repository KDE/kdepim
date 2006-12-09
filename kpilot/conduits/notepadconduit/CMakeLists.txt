include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(conduit_notepad_SRCS
	notepad-factory.cc
	notepad-conduit.cc
)

set(conduit_notepad_UIS
	notepad-setup.ui
)

set(conduit_notepad_KCFGS
	notepadconduit.kcfgc
)

kde3_add_kcfg_files(conduit_notepad_SRCS ${conduit_notepad_KCFGS})
kde3_add_ui_files(conduit_notepad_SRCS ${conduit_notepad_UIS})
kde3_automoc(${conduit_notepad_SRCS})
add_library(conduit_notepad SHARED ${conduit_notepad_SRCS})

set_target_properties(
	conduit_notepad PROPERTIES LOCATION ${KDE3_PLUGIN_INSTALL_DIR}
	INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib
	PREFIX ""
)

kde3_install_libtool_file(conduit_notepad)

install(
	TARGETS conduit_notepad
	LIBRARY DESTINATION ${KDE3_PLUGIN_INSTALL_DIR}
)

install(
	FILES notepad-conduit.desktop DESTINATION ${KDE3_SERVICES_DIR}
)
