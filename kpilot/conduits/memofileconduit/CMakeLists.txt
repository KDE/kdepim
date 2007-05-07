include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(conduit_memofile_SRCS
	memofile-factory.cc
	memofile.cc
	memofiles.cc
	memofile-conduit.cc
)

set(conduit_memofile_UIS
	setup_base.ui
)

set(conduit_memofile_KCFGS
	memofileSettings.kcfgc
)

kde4_add_kcfg_files(conduit_memofile_SRCS ${conduit_memofile_KCFGS})
kde4_add_ui3_files(conduit_memofile_SRCS ${conduit_memofile_UIS})
kde4_automoc(${conduit_memofile_SRCS})
kde4_add_plugin(conduit_memofile WITH_PREFIX ${conduit_memofile_SRCS})
target_link_libraries(conduit_memofile ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} kpilot)
install(TARGETS conduit_memofile DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES memofile-conduit.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES memofileconduit.kcfg DESTINATION ${KCFG_INSTALL_DIR})
