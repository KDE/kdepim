include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(kpilot_conduit_abbrowser_SRCS
	resolutionDialog.cc
	abbrowser-factory.cc
	abbrowser-setup.cc
	kabcRecord.cc
	abbrowser-conduit.cc
)

set(kpilot_conduit_abbrowser_UIS
	resolutionDialog_base.ui
	kaddressbookConduit.ui
)

set(kpilot_conduit_abbrowser_KCFGS
	abbrowserSettings.kcfgc
)

kde4_add_kcfg_files(kpilot_conduit_abbrowser_SRCS ${kpilot_conduit_abbrowser_KCFGS})
kde4_add_ui3_files(kpilot_conduit_abbrowser_SRCS ${kpilot_conduit_abbrowser_UIS})
kde4_add_plugin(kpilot_conduit_address ${kpilot_conduit_abbrowser_SRCS})
target_link_libraries(kpilot_conduit_address kabc_file_core ${KDE4_KABC_LIBS} ${KDE4_KDEUI_LIBS} kpilot)
install(TARGETS kpilot_conduit_address DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES abbrowser_conduit.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES abbrowserconduit.kcfg DESTINATION ${KCFG_INSTALL_DIR})

