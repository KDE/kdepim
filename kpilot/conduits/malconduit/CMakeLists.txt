include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${MAL_INCLUDE_DIR}
)

set(kpilot_conduit_mal_SRCS
	mal-factory.cc
	mal-setup.cc
	mal-conduit.cc
)

set(kpilot_conduit_mal_UIS
	mal-setup_dialog.ui
)

set(kpilot_conduit_mal_KCFGS
	malconduitSettings.kcfgc
)

kde4_add_kcfg_files(kpilot_conduit_mal_SRCS ${kpilot_conduit_mal_KCFGS})
kde4_add_ui3_files(kpilot_conduit_mal_SRCS ${kpilot_conduit_mal_UIS})
kde4_add_plugin(kpilot_conduit_mal ${kpilot_conduit_mal_SRCS})
target_link_libraries(kpilot_conduit_mal ${MAL_LIBRARY} ${KDE4_UI_LIBS} ${QT_LIBRARIES} kpilot)

#install(TARGETS kpilot_conduit_mal DESTINATION ${PLUGIN_INSTALL_DIR})
#install(FILES mal_conduit.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#install(FILES malconduit.kcfg DESTINATION ${KCFG_INSTALL_DIR})

