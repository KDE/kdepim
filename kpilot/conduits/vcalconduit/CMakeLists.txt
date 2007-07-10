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

kde4_add_kcfg_files(conduit_SHARED vcalconduitSettings.kcfgc)
kde4_add_ui3_files(conduit_SHARED korganizerConduit.ui)

set(conduit_vcal_SRCS
	${conduit_SHARED}
	vcal-conduit.cc
	vcal-factory.cc
	vcal-setup.cc
)

kde4_add_library(conduit_vcal SHARED ${conduit_vcal_SRCS})
target_link_libraries(conduit_vcal ${QT_LIBRARIES}  ${KDE4_KCAL_LIBS} ${KDE4_KRESOURCES_LIBS} kpilot)

set(conduit_todo_SRCS
	${conduit_SHARED}
	todo-factory.cc
	todo-setup.cc
	todo-conduit.cc
)

kde4_add_library(conduit_todo SHARED ${conduit_todo_SRCS})
target_link_libraries(conduit_todo ${QT_LIBRARIES} ${KDE4_KCAL_LIBS} ${KDE4_KRESOURCES_LIBS} kpilot)

set_target_properties(
	conduit_vcal PROPERTIES LOCATION ${PLUGIN_INSTALL_DIR}
	INSTALL_RPATH ${LIB_INSTALL_DIR}
	PREFIX "lib"
)
set_target_properties(
	conduit_todo PROPERTIES LOCATION ${PLUGIN_INSTALL_DIR}
	INSTALL_RPATH ${LIB_INSTALL_DIR}
	PREFIX "lib"
)

install(
	TARGETS conduit_vcal conduit_todo
	LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
	LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
)

install(
	FILES vcal-conduit.desktop todo-conduit.desktop
	DESTINATION ${SERVICES_INSTALL_DIR}
)

install(FILES vcalconduitbase.kcfg DESTINATION ${KCFG_INSTALL_DIR})
