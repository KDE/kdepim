include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../base
	${CMAKE_CURRENT_BINARY_DIR}
)

set(contacts_SRCS
	contacts-setup.cc
	hhcontact.cc
	akonadicontact.cc
	contactshhdataproxy.cc
	contactsakonadidataproxy.cc
	contacts.cc
	contacts-factory.cc
)

set(contacts_KCFGS
	contactsSettings.kcfgc
)

kde4_add_kcfg_files(contacts_SRCS ${contacts_KCFGS})
kde4_add_ui_files(contacts_SRCS contacts-settings.ui)
kde4_add_library(kpilot_conduit_contacts SHARED ${contacts_SRCS})

target_link_libraries(kpilot_conduit_contacts
	kpilot_conduit_base
	${QT_LIBRARIES}
	${KDE4_AKONADI_LIBS}
)

set_target_properties(kpilot_conduit_contacts PROPERTIES PREFIX "")

install(TARGETS kpilot_conduit_contacts LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES contacts-conduit.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES contacts.kcfg DESTINATION ${KCFG_INSTALL_DIR})
