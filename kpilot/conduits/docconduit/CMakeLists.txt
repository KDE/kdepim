include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(conduit_doc_SRCS
	makedoc9.cc
	pilotDOCHead.cc
	pilotDOCEntry.cc
	pilotDOCBookmark.cc
	DOC-converter.cc
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

install(
	FILES doc_conduit.desktop DESTINATION ${KDE3_SERVICES_DIR}
)

install(
	FILES docconduit.kcfg kpalmdoc.kcfg DESTINATION ${KDE3_KCFG_DIR}
)

install(
	FILES kpalmdoc.upd
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/apps/kconf_update
)

add_subdirectory(Icons)
