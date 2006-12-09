include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(conduit_null_SRCS
	null-conduit.cc
	null-factory.cc
)

set(conduit_null_UIS
	setup_base.ui
)

set(conduit_null_KCFGS
	nullSettings.kcfgc
)

kde3_add_kcfg_files(conduit_null_SRCS ${conduit_null_KCFGS})
kde3_add_ui_files(conduit_null_SRCS ${conduit_null_UIS})
kde3_automoc(${conduit_null_SRCS})
add_library(conduit_null SHARED ${conduit_null_SRCS})

set_target_properties(
	conduit_null PROPERTIES LOCATION ${KDE3_PLUGIN_INSTALL_DIR}
	INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib
	PREFIX ""
)

kde3_install_libtool_file(conduit_null)

install(
	TARGETS conduit_null
	LIBRARY DESTINATION ${KDE3_PLUGIN_INSTALL_DIR}
)

install(
	FILES null-conduit.desktop DESTINATION ${KDE3_SERVICES_DIR}
)
