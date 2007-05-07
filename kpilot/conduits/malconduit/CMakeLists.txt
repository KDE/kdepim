include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${MAL_INCLUDE_DIR}
)

set(conduit_mal_SRCS
	mal-factory.cc 
	mal-setup.cc 
	mal-conduit.cc 
)

set(conduit_mal_UIS
	mal-setup_dialog.ui
)

set(conduit_mal_KCFGS
	malconduitSettings.kcfgc
)

kde4_add_kcfg_files(conduit_mal_SRCS ${conduit_mal_KCFGS})
kde4_add_ui3_files(conduit_mal_SRCS ${conduit_mal_UIS})
kde4_automoc(${conduit_mal_SRCS})
#target_link_libraries(conduit_mal ${MAL_LIBRARY})
kde4_add_plugin(conduit_mal WITH_PREFIX ${conduit_mal_SRCS})
install(TARGETS conduit_mal DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES mal_conduit.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES malconduit.kcfg DESTINATION ${KCFG_INSTALL_DIR})

