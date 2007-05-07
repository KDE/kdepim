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

set(conduit_doc_SRCS
	${doc_shared_SRCS}
	kpalmdoc_dlg.cc
	kpalmdoc.cpp
	doc-factory.cc
	doc-setup.cc
	doc-conduit.cc
	doc-conflictdialog.cc
)

set(conduit_doc_UIS
	kpalmdoc_dlgbase.ui
	doc-setupdialog.ui
)

set(conduit_doc_KCFGS
	docconduitSettings.kcfgc
	kpalmdocSettings.kcfgc
)

kde4_add_kcfg_files(conduit_doc_SRCS ${conduit_doc_KCFGS})
kde4_add_ui3_files(conduit_doc_SRCS ${conduit_doc_UIS})
kde4_automoc(${conduit_doc_SRCS})
kde4_add_plugin(conduit_doc WITH_PREFIX ${conduit_doc_SRCS})
target_link_libraries(conduit_doc ${QT_LIBRARIES} kpilot ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})
install(TARGETS conduit_doc DESTINATION ${PLUGIN_INSTALL_DIR})

set(kpalmdoc_SRCS
	${doc_shared_SRCS}
	kpalmdoc_dlg.cc
	kpalmdoc.cpp
)
kde4_add_kcfg_files(kpalmdoc_SRCS kpalmdocSettings.kcfgc)
kde4_add_ui3_files(kpalmdoc_SRCS kpalmdoc_dlgbase.ui)
kde4_automoc(${kpalmdoc_SRCS})
kde4_add_executable(kpalmdoc ${kpalmdoc_SRCS})
target_link_libraries(kpalmdoc ${QT_LIBRARIES} kpilot ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})

install(
	TARGETS kpalmdoc conduit_doc
	LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

install(
	FILES doc_conduit.desktop DESTINATION ${SERVICES_INSTALL_DIR}
)

install(
	FILES docconduit.kcfg kpalmdoc.kcfg DESTINATION ${KCFG_INSTALL_DIR}
)

install(
	FILES kpalmdoc.desktop DESTINATION ${XDG_APPS_DIR}
)

install(
	FILES kpalmdoc.upd
	DESTINATION ${KCONF_UPDATE_INSTALL_DIR}
)

add_subdirectory(icons)
