include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../akonadibase
	${CMAKE_CURRENT_SOURCE_DIR}/../base
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../akonadibase
)

set( todo_SRCS
	todoconfig.cc
	todoconduit.cc
	todofactory.cc
)

kde4_add_ui_files( todo_SRCS settingswidget.ui )
kde4_add_kcfg_files( todo_SRCS todosettings.kcfgc )
kde4_add_library( kpilot_conduit_todo2 SHARED ${todo_SRCS} )

target_link_libraries( kpilot_conduit_todo2
	kpilot_akonadibase
)

set_target_properties( kpilot_conduit_todo2 PROPERTIES PREFIX "" )

install( TARGETS kpilot_conduit_todo2 LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kpilot-conduit-todo.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES todosettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
