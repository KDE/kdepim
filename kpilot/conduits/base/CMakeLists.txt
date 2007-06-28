include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(recordConduit_SRCS
	#RecordConduit.cpp
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
#kde4_add_library(recordConduit SHARED ${recordConduit_SRCS})
#target_link_libraries(recordConduit ${QT_LIBRARIES} kpilot)

#install(TARGETS recordConduit LIBRARY DESTINATION ${LIB_INSTALL_DIR})
