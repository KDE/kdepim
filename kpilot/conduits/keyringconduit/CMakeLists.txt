include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../base
	${CMAKE_CURRENT_BINARY_DIR}
	${QCA2_INCLUDE_DIR}
)

add_subdirectory(keyringeditor)

set(keyring_SRCS
	pi-keyring.cc
	keyringsetup.cc
	keyringhhrecord.cc
	keyringhhdataproxy.cc
	keyringconduit.cc
	keyringfactory.cc
)

set(keyring_KCFGS
	keyringsettings.kcfgc
)

kde4_add_kcfg_files(keyring_SRCS ${keyring_KCFGS})
kde4_add_ui_files(keyring_SRCS keyringwidget.ui)
kde4_add_library(kpilot_conduit_keyring SHARED ${keyring_SRCS})

target_link_libraries(kpilot_conduit_keyring
	kpilot_conduit_base
	${QT_LIBRARIES}
	${QCA2_LIBRARIES}
)

set_target_properties(kpilot_conduit_keyring PROPERTIES PREFIX "")

install(TARGETS kpilot_conduit_keyring LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kpilot-conduit-keyring.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES keyringconduit.kcfg DESTINATION ${KCFG_INSTALL_DIR})
