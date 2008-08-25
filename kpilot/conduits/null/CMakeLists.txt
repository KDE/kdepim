include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../lib
)

set(kpilot_conduit_null_SRCS
	null-conduit.cc
	null-factory.cc
)

set(kpilot_conduit_null_UIS
	setup_base.ui
)

set(kpilot_conduit_null_KCFGS
	nullSettings.kcfgc
)

kde4_add_kcfg_files(kpilot_conduit_null_SRCS ${kpilot_conduit_null_KCFGS})
kde4_add_ui_files(kpilot_conduit_null_SRCS ${kpilot_conduit_null_UIS})

kde4_add_plugin(kpilot_conduit_null ${kpilot_conduit_null_SRCS})
target_link_libraries(kpilot_conduit_null ${KDE4_KDEUI_LIBS} kpilot)
install(TARGETS kpilot_conduit_null DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES null-conduit.desktop DESTINATION ${SERVICES_INSTALL_DIR})
