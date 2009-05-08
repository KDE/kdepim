include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../akonadibase
	${CMAKE_CURRENT_SOURCE_DIR}/../base
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../akonadibase
	${CMAKE_CURRENT_BINARY_DIR}/../../lib
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

set( contacts_SRCS
	contactsakonadirecord.cc
	contactsakonadiproxy.cc
	contactsconduit.cc
	contactsconfig.cc
	contactshhdataproxy.cc
	contactshhrecord.cc
	contactsfactory.cc
)

kde4_add_ui_files( contacts_SRCS settingswidget.ui )
kde4_add_kcfg_files( contacts_SRCS contactssettings.kcfgc )
kde4_add_library( kpilot_conduit_contacts SHARED ${contacts_SRCS} )

target_link_libraries( kpilot_conduit_contacts
	${KDEPIMLIBS_KCAL_LIBS} ${KDEPIMLIBS_AKONADI_LIBS}
	kpilot_akonadibase kpilot_conduit_base kpilot
)

set_target_properties( kpilot_conduit_contacts PROPERTIES PREFIX "" )

install( TARGETS kpilot_conduit_contacts LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kpilot-conduit-contacts.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES contactssettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
