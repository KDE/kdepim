include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../lib
)

set(kpilot_conduit_notepad_SRCS
	notepad-factory.cc
	notepad-conduit.cc
)

set(kpilot_conduit_notepad_UIS
	notepad-setup.ui
)

set(kpilot_conduit_notepad_KCFGS
	notepadconduit.kcfgc
)

kde4_add_kcfg_files(kpilot_conduit_notepad_SRCS ${kpilot_conduit_notepad_KCFGS})
kde4_add_ui_files(kpilot_conduit_notepad_SRCS ${kpilot_conduit_notepad_UIS})
kde4_add_plugin(kpilot_conduit_notepad ${kpilot_conduit_notepad_SRCS})
target_link_libraries(kpilot_conduit_notepad  ${KDE4_KIO_LIBS} kpilot)

install(TARGETS kpilot_conduit_notepad DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES notepad-conduit.desktop DESTINATION ${SERVICES_INSTALL_DIR})
