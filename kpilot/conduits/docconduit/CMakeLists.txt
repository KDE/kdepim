include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(doc_shared_SRCS
	makedoc9.cc
	pilotDOCHead.cc
	pilotDOCEntry.cc
	pilotDOCBookmark.cc
	DOC-converter.cc
)

set(kpilot_conduit_doc_SRCS
	${doc_shared_SRCS}
	doc-factory.cc
	doc-setup.cc
	doc-conduit.cc
	doc-conflictdialog.cc
)

set(kpilot_conduit_doc_UIS
	doc-setupdialog.ui
)

set(kpilot_conduit_doc_KCFGS
	docconduitSettings.kcfgc
)

kde4_add_kcfg_files(kpilot_conduit_doc_SRCS ${kpilot_conduit_doc_KCFGS})
kde4_add_ui_files(kpilot_conduit_doc_SRCS ${kpilot_conduit_doc_UIS})
kde4_add_plugin(kpilot_conduit_doc ${kpilot_conduit_doc_SRCS})
target_link_libraries(kpilot_conduit_doc ${QT_LIBRARIES} kpilot  ${KDE4_KIO_LIBS})
#install(TARGETS kpilot_conduit_doc DESTINATION ${PLUGIN_INSTALL_DIR})

set(kpalmdoc_SRCS
	${doc_shared_SRCS}
	kpalmdoc_dlg.cc
	kpalmdoc.cpp
)
kde4_add_kcfg_files(kpalmdoc_SRCS kpalmdocSettings.kcfgc)
kde4_add_ui3_files(kpalmdoc_SRCS kpalmdoc_dlgbase.ui)
kde4_add_executable(kpalmdoc ${kpalmdoc_SRCS})
target_link_libraries(kpalmdoc ${QT_LIBRARIES} kpilot  ${KDE4_KIO_LIBS})

#install(
#	TARGETS kpalmdoc ${INSTALL_TARGETS_DEFAULT_ARGS}) 

#install(
#	FILES doc_conduit.desktop DESTINATION ${SERVICES_INSTALL_DIR}
#)

#install(
#	FILES docconduit.kcfg kpalmdoc.kcfg DESTINATION ${KCFG_INSTALL_DIR}
#)

#install(
#	FILES kpalmdoc.desktop DESTINATION ${XDG_APPS_INSTALL_DIR}
#)

#install(
#	FILES kpalmdoc.upd
#	DESTINATION ${KCONF_UPDATE_INSTALL_DIR}
#)

add_subdirectory(icons)
