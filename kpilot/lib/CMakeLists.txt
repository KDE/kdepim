include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(lib_SRCS
	options.cc
	syncAction.cc
	plugin.cc
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

configure_file (kpilot_export.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kpilot_export.h )


kde4_add_kcfg_files(lib_SRCS kpilotlibSettings.kcfgc)
kde4_add_library(kpilot SHARED ${lib_SRCS})
set_target_properties(kpilot PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
set_target_properties(kpilot PROPERTIES LINK_INTERFACE_LIBRARIES "${QT_QTCORE_LIBRARY};${QT_QT3SUPPORT_LIBRARY};${PILOTLINK_LIBRARY}")

target_link_libraries(kpilot
	${PILOTLINK_LIBRARY}
	${QT_LIBRARIES}
	${KDE4_KIO_LIBS}
)

#---------- INSTALL -----------------------*
set(kpilotinclude_HEADERS
	kpilotlink.h
	${CMAKE_CURRENT_BINARY_DIR}/kpilot_export.h
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

install(TARGETS kpilot LIBRARY ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES ${kpilotinclude_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/kpilot)

install(FILES kpilotlib.kcfg DESTINATION ${KCFG_INSTALL_DIR})

