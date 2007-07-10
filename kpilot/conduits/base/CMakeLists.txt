add_subdirectory(tests)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(conduit_base_SRCS
	idmappingxmlsource.cc
	idmapping.cc
	cudcounter.cc
	record.cc
	dataproxy.cc
	pcdataproxy.cc
	hhdataproxy.cc
	recordconduit.cc
)

set(conduit_base_KCFGS
	recordconduitSettings.kcfgc
)

kde4_add_kcfg_files(conduit_base_SRCS ${conduit_base_KCFGS})
kde4_add_library(conduit_base SHARED ${conduit_base_SRCS})
target_link_libraries(conduit_base ${QT_LIBRARIES} kpilot)

#install(TARGETS recordConduit LIBRARY DESTINATION ${LIB_INSTALL_DIR})
