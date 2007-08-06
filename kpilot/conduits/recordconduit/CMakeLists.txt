include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(recordconduit_SRCS
	factory.cc
)

set(recordconduit_UIS
	setup_base.ui
)

set(recordconduit_KCFGS
	settings.kcfgc
)

kde4_add_kcfg_files(recordconduit_SRCS ${recordconduit_KCFGS})
kde4_add_ui3_files(recordconduit_SRCS ${recordconduit_UIS})
kde4_add_plugin(kpilot_conduit_record ${recordconduit_SRCS})
target_link_libraries(kpilot_conduit_record  ${KDE4_KIO_LIBS} kpilot)
install(TARGETS kpilot_conduit_record DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES record-conduit.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES settings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
