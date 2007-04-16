set(conduit_LIBS kcal)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(conduit_SHARED
	vcal-setupbase.cc
	vcal-conduitbase.cc
	teststate.cc
	initstate.cc
	pctohhstate.cc
	hhtopcstate.cc
	cleanupstate.cc
	deleteunsyncedpcstate.cc
	deleteunsyncedhhstate.cc
	kcalRecord.cc
	vcalRecord.cc
	todoRecord.cc
)

kde3_add_kcfg_files(conduit_SHARED vcalconduitSettings.kcfgc)
kde3_add_ui_files(conduit_SHARED korganizerConduit.ui)

set(conduit_vcal_SRCS
	${conduit_SHARED}
	vcal-conduit.cc
	vcal-factory.cc
	vcal-setup.cc
)

kde3_automoc(${conduit_vcal_SRCS})
add_library(conduit_vcal SHARED ${conduit_vcal_SRCS})
target_link_libraries(conduit_vcal kcal)

set(conduit_todo_SRCS
	${conduit_SHARED}
	todo-factory.cc
	todo-setup.cc
	todo-conduit.cc
)

kde3_automoc(${conduit_todo_SRCS})
add_library(conduit_todo SHARED ${conduit_todo_SRCS})
target_link_libraries(conduit_todo kcal)

set_target_properties(
	conduit_vcal PROPERTIES LOCATION ${KDE3_PLUGIN_INSTALL_DIR}
	INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib
	PREFIX ""
)
set_target_properties(
	conduit_todo PROPERTIES LOCATION ${KDE3_PLUGIN_INSTALL_DIR}
	INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib
	PREFIX ""
)

kde3_install_libtool_file(conduit_vcal)

install(
	TARGETS conduit_vcal conduit_todo
	LIBRARY DESTINATION ${KDE3_PLUGIN_INSTALL_DIR}
	LIBRARY DESTINATION ${KDE3_PLUGIN_INSTALL_DIR}
)

kde3_install_libtool_file(conduit_todo)

install(
	FILES vcal-conduit.desktop todo-conduit.desktop
	DESTINATION ${KDE3_SERVICES_DIR}
)

install(
	FILES vcalconduitbase.kcfg DESTINATION ${KDE3_KCFG_DIR}
)
