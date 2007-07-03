include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(conduit_base_SRCS
	idmapping.cc
	cudcounter.cc
	#record.cc
	dataproxy.cc
	pcdataproxy.cc
	hhdataproxy.cc
	recordconduit.cc
)

#set(conduit_mal_UIS
#	mal-setup_dialog.ui
#)

#set(conduit_mal_KCFGS
#	malconduitSettings.kcfgc
#)

#kde4_add_kcfg_files(conduit_mal_SRCS ${conduit_mal_KCFGS})
#kde4_add_ui3_files(conduit_mal_SRCS ${conduit_mal_UIS})
#kde4_automoc(${recordConduit_SRCS})
#kde4_add_plugin(conduit_mal WITH_PREFIX ${conduit_mal_SRCS})
kde4_add_library(conduit_base SHARED ${conduit_base_SRCS})
target_link_libraries(conduit_base ${QT_LIBRARIES} kpilot)

#install(TARGETS recordConduit LIBRARY DESTINATION ${LIB_INSTALL_DIR})
