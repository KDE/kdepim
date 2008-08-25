include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../lib
)

set(kpilot_conduit_memofile_SRCS
	memofile-factory.cc
	memofile.cc
	memofiles.cc
	memofile-conduit.cc
)

set(kpilot_conduit_memofile_UIS
	setup_base.ui
)

set(kpilot_conduit_memofile_KCFGS
	memofileSettings.kcfgc
)

kde4_add_kcfg_files(kpilot_conduit_memofile_SRCS ${kpilot_conduit_memofile_KCFGS})
kde4_add_ui_files(kpilot_conduit_memofile_SRCS ${kpilot_conduit_memofile_UIS})
kde4_add_plugin(kpilot_conduit_memofile ${kpilot_conduit_memofile_SRCS})
target_link_libraries(kpilot_conduit_memofile  ${KDE4_KIO_LIBS} kpilot)
install(TARGETS kpilot_conduit_memofile DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES memofile-conduit.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES memofileconduit.kcfg DESTINATION ${KCFG_INSTALL_DIR})
