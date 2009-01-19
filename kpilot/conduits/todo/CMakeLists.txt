include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../akonadibase
	${CMAKE_CURRENT_SOURCE_DIR}/../base
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../akonadibase
	${CMAKE_CURRENT_BINARY_DIR}/../../lib
	${Boost_INCLUDE_DIR} 
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

set( todo_SRCS
	todoakonadirecord.cc
	todoakonadiproxy.cc
	todoconduit.cc
	todoconfig.cc
	todohhdataproxy.cc
	todohhrecord.cc
	todofactory.cc
)

kde4_add_ui_files( todo_SRCS settingswidget.ui )
kde4_add_kcfg_files( todo_SRCS todosettings.kcfgc )
kde4_add_library( kpilot_conduit_todo SHARED ${todo_SRCS} )

target_link_libraries( kpilot_conduit_todo
	${KDE4_KCAL_LIBS} ${KDE4_AKONADI_LIBS}
	kpilot_akonadibase kpilot_conduit_base kpilot
)

set_target_properties( kpilot_conduit_todo PROPERTIES PREFIX "" )

install( TARGETS kpilot_conduit_todo LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kpilot-conduit-todo.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES todosettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
