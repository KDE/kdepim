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

kde3_add_kcfg_files(conduit_mal_SRCS ${conduit_mal_KCFGS})
kde3_add_ui_files(conduit_mal_SRCS ${conduit_mal_UIS})
kde3_automoc(${conduit_mal_SRCS})
add_library(conduit_mal SHARED ${conduit_mal_SRCS})
target_link_libraries(conduit_mal ${MAL_LIBRARY})

set_target_properties(
	conduit_mal PROPERTIES 
	LOCATION ${KDE3_PLUGIN_INSTALL_DIR}
	PREFIX ""
	INSTALL_RPATH "${MAL_LIBRARY}"
	INSTALL_RPATH_USE_LINK_PATH true
)

kde3_install_libtool_file(conduit_mal)

install(
	TARGETS conduit_mal
	LIBRARY DESTINATION ${KDE3_PLUGIN_INSTALL_DIR}
)

install(
	FILES mal_conduit.desktop DESTINATION ${KDE3_SERVICES_DIR}
)

install(
	FILES malconduit.kcfg DESTINATION ${KDE3_KCFG_DIR}
)

