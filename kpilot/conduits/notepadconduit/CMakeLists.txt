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

kde4_add_kcfg_files(conduit_notepad_SRCS ${conduit_notepad_KCFGS})
kde4_add_ui_files(conduit_notepad_SRCS ${conduit_notepad_UIS})
kde4_automoc(${conduit_notepad_SRCS})
kde4_add_plugin(conduit_notepad WITH_PREFIX ${conduit_notepad_SRCS})
target_link_libraries(conduit_notepad ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} kpilot)

install(TARGETS conduit_notepad DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES notepad-conduit.desktop DESTINATION ${SERVICES_INSTALL_DIR})
