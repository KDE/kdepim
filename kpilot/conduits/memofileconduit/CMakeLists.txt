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

kde3_add_kcfg_files(conduit_memofile_SRCS ${conduit_memofile_KCFGS})
kde3_add_ui_files(conduit_memofile_SRCS ${conduit_memofile_UIS})
kde3_automoc(${conduit_memofile_SRCS})
add_library(conduit_memofile SHARED ${conduit_memofile_SRCS})

set_target_properties(
	conduit_memofile PROPERTIES LOCATION ${KDE3_PLUGIN_INSTALL_DIR}
	INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib
	PREFIX ""
)

kde3_install_libtool_file(conduit_memofile)

install(
	TARGETS conduit_memofile
	LIBRARY DESTINATION ${KDE3_PLUGIN_INSTALL_DIR}
)

install(
	FILES memofile-conduit.desktop DESTINATION ${KDE3_SERVICES_DIR}
)

install(
	FILES memofileconduit.kcfg DESTINATION ${KDE3_KCFG_DIR}
)
