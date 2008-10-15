include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../base
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../lib
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

set( memofile_SRCS
	memofileconduit.cc
	memofileconfig.cc
	memofilefactory.cc
	memofilehhdataproxy.cc
	memofilehhrecord.cc
	memofilepcdataproxy.cc
	memofilepcrecord.cc
)

kde4_add_ui_files( memofile_SRCS settingswidget.ui )
kde4_add_kcfg_files( memofile_SRCS settings.kcfgc )
kde4_add_library( kpilot_conduit_memofile SHARED ${memofile_SRCS} )

target_link_libraries( kpilot_conduit_memofile
	kpilot_conduit_base kpilot
)

set_target_properties( kpilot_conduit_memofile PROPERTIES PREFIX "" )

install( TARGETS kpilot_conduit_memofile LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kpilot-conduit-memofile.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES memofilesettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
