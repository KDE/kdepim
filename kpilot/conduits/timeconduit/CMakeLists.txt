include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(conduit_time_SRCS
	time-conduit.cc
	time-factory.cc
	time-setup.cc
)

set(conduit_time_UIS
	time-setup_dialog.ui
)

set(conduit_time_KCFGS
	timeConduitSettings.kcfgc
)

kde3_add_kcfg_files(conduit_time_SRCS ${conduit_time_KCFGS})
kde3_add_ui_files(conduit_time_SRCS ${conduit_time_UIS})
kde3_automoc(${conduit_time_SRCS})
add_library(conduit_time SHARED ${conduit_time_SRCS})

kpilot_rpath(conduit_time)

set_target_properties(
	conduit_time PROPERTIES LOCATION ${KDE3_PLUGIN_INSTALL_DIR}
	PREFIX ""
)

kde3_install_libtool_file(conduit_time)

install(
	TARGETS conduit_time
	LIBRARY DESTINATION ${KDE3_PLUGIN_INSTALL_DIR}
)

install(
	FILES time_conduit.desktop DESTINATION ${KDE3_SERVICES_DIR}
)

install(
	FILES timeconduit.kcfg DESTINATION ${KDE3_KCFG_DIR}
)
