add_subdirectory(tests)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_BINARY_DIR}/kpilot
	${CMAKE_CURRENT_BINARY_DIR}/../../lib
)

# A bit ugly? Maybe settings should be shared.
set(settings_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/../../kpilot/kpilotConfig.cc
)

kde4_add_kcfg_files(settings_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/../../kpilot/kpilotSettings.kcfgc)

set(conduit_base_SRCS
	${settings_SRC}
	idmappingxmlsource.cc
	idmapping.cc
	cudcounter.cc
	hhrecord.cc
	dataproxy.cc
	hhdataproxy.cc
	recordconduit.cc
)

set(conduit_base_KCFGS
	recordconduitSettings.kcfgc
)

kde4_add_kcfg_files(conduit_base_SRCS ${conduit_base_KCFGS})
kde4_add_library(kpilot_conduit_base SHARED ${conduit_base_SRCS})
target_link_libraries(kpilot_conduit_base ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} kpilot)

install(TARGETS kpilot_conduit_base LIBRARY DESTINATION ${LIB_INSTALL_DIR})
