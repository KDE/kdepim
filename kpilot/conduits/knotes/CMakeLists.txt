include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(conduit_knotes_SRCS
	knotes-factory.cc
	knotes-setup.cc
	knotes-action.cc
)

set(conduit_knotes_UIS
	setup_base.ui
)

set(conduit_knotes_KCFGS
	knotesconduitSettings.kcfgc
)

#set(conduit_knotes_STUBS
#	KNotesIface.h
#)

#kde3_add_dcop_skels(conduit_knotes_SRCS ${conduit_knotes_STUBS})
#kde3_add_dcop_stubs(conduit_knotes_SRCS ${conduit_knotes_STUBS})

kde3_add_kcfg_files(conduit_knotes_SRCS ${conduit_knotes_KCFGS})
kde3_add_ui_files(conduit_knotes_SRCS ${conduit_knotes_UIS})
kde3_automoc(${conduit_knotes_SRCS})

add_library(conduit_knotes SHARED ${conduit_knotes_SRCS})
target_link_libraries(conduit_knotes kcal)

set_target_properties(conduit_knotes PROPERTIES
	LOCATION ${KDE3_PLUGIN_INSTALL_DIR}
	INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib
	PREFIX ""
)

kde3_install_libtool_file(conduit_knotes)

install(
	TARGETS conduit_knotes
	LIBRARY DESTINATION ${KDE3_PLUGIN_INSTALL_DIR}
)

install(
	FILES knotes-conduit.desktop DESTINATION ${KDE3_SERVICES_DIR}
)

install(
	FILES knotesconduit.kcfg DESTINATION ${KDE3_KCFG_DIR}
)
