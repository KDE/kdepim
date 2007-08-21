include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(lib_SRCS
	options.cc
	plugin.cc
	syncAction.cc
	actions.cc
	actionQueue.cc
	kpilotlink.cc
	kpilotdevicelink.cc
	kpilotlocallink.cc
	pilot.cc
	pilotAppInfo.cc
	pilotRecord.cc
	pilotDatabase.cc
	pilotLocalDatabase.cc
	pilotSerialDatabase.cc
	pilotMemo.cc
	pilotAddress.cc
	pilotDateEntry.cc
	pilotTodoEntry.cc
)

kde4_add_kcfg_files(lib_SRCS kpilotlibSettings.kcfgc)
kde4_add_library(kpilot SHARED ${lib_SRCS})
set_target_properties(kpilot PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

target_link_libraries(kpilot
	${PILOTLINK_LIBRARY}
	${QT_LIBRARIES}
	${KDE4_KIO_LIBS}
)

#---------- INSTALL -----------------------*
set(kpilotinclude_HEADERS
	kpilotlink.h
	kpilot_export.h
	kpilotdevicelink.h
	kpilotlocallink.h
	pilot.h
	pilotDatabase.h
	pilotLinkVersion.h
	pilotLocalDatabase.h
	pilotRecord.h
	pilotSerialDatabase.h
	plugin.h
	pluginfactory.h
	syncAction.h
	options.h
)

install(TARGETS kpilot LIBRARY DESTINATION ${LIB_INSTALL_DIR})

install(FILES ${kpilotinclude_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/kpilot)

install(FILES kpilotlib.kcfg DESTINATION ${KCFG_INSTALL_DIR})

