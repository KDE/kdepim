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

kde3_add_kcfg_files(conduit_doc_SRCS ${conduit_doc_KCFGS})
kde3_add_ui_files(conduit_doc_SRCS ${conduit_doc_UIS})
kde3_automoc(${conduit_doc_SRCS})
add_library(conduit_doc SHARED ${conduit_doc_SRCS})

kpilot_rpath(conduit_doc)

set_target_properties(
	conduit_doc PROPERTIES LOCATION ${KDE3_PLUGIN_INSTALL_DIR}
	PREFIX ""
)

kde3_install_libtool_file(conduit_doc)

install(
	TARGETS conduit_doc
	LIBRARY DESTINATION ${KDE3_PLUGIN_INSTALL_DIR}
)

set(kpalmdoc_SRCS
	${doc_shared_SRCS}
	kpalmdoc_dlg.cc
	kpalmdoc.cpp
)
kde3_add_kcfg_files(kpalmdoc_SRCS kpalmdocSettings.kcfgc)
kde3_add_ui_files(kpalmdoc_SRCS kpalmdoc_dlgbase.ui)
kde3_automoc(${kpalmdoc_SRCS})
add_executable(kpalmdoc ${kpalmdoc_SRCS})
target_link_libraries(kpalmdoc ${QT_LIBRARIES} kpilot kdeui kio)
kpilot_rpath(kpalmdoc)

install(
	TARGETS kpalmdoc conduit_doc
	LIBRARY DESTINATION ${KDE3_PLUGIN_INSTALL_DIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

install(
	FILES doc_conduit.desktop DESTINATION ${KDE3_SERVICES_DIR}
)

install(
	FILES docconduit.kcfg kpalmdoc.kcfg DESTINATION ${KDE3_KCFG_DIR}
)

install(
	FILES kpalmdoc.desktop DESTINATION ${KDE3_XDG_APPS_DIR}
)

install(
	FILES kpalmdoc.upd
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/apps/kconf_update
)

add_subdirectory(Icons)
