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

kde4_add_kcfg_files(conduit_null_SRCS ${conduit_null_KCFGS})
kde4_add_ui3_files(conduit_null_SRCS ${conduit_null_UIS})
kde4_automoc(${conduit_null_SRCS})

kde4_add_plugin(conduit_null WITH_PREFIX ${conduit_null_SRCS})
target_link_libraries(conduit_null ${KDE4_KDEUI_LIBS} kpilot)
install(TARGETS conduit_null DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES null-conduit.desktop DESTINATION ${SERVICES_INSTALL_DIR})
