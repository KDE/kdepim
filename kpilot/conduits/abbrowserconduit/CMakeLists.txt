include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(conduit_abbrowser_SRCS
	resolutionDialog.cc
	abbrowser-factory.cc
	abbrowser-setup.cc
	kabcRecord.cc
	abbrowser-conduit.cc
)

set(conduit_abbrowser_UIS
	resolutionDialog_base.ui
	kaddressbookConduit.ui
)

set(conduit_abbrowser_KCFGS
	abbrowserSettings.kcfgc
)

kde4_add_kcfg_files(conduit_abbrowser_SRCS ${conduit_abbrowser_KCFGS})
kde4_add_ui3_files(conduit_abbrowser_SRCS ${conduit_abbrowser_UIS})
kde4_add_plugin(conduit_address WITH_PREFIX ${conduit_abbrowser_SRCS})
target_link_libraries(conduit_address kabc_file_core ${KDE4_KABC_LIBS} ${KDE4_KDEUI_LIBS} kpilot)
install(TARGETS conduit_address DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES abbrowser_conduit.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES abbrowserconduit.kcfg DESTINATION ${KCFG_INSTALL_DIR})

