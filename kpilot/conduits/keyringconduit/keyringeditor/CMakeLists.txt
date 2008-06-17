include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../
	${CMAKE_CURRENT_BINARY_DIR}
)

set(tests_SRCS
	localkeyringproxy.cc
	keyringlistmodel.cc
	passworddialog.cc
	keyringviewer.cc
	main.cc
	../pi-keyring.cc
	../keyringconduit.cc
	../keyringhhdataproxy.cc
	../keyringhhrecord.cc
)

kde4_add_kcfg_files(tests_SRCS ../keyringsettings.kcfgc)
kde4_add_ui_files( tests_SRCS 
	viewer.ui
	passworddialog.ui
)
kde4_add_executable(keyringeditor ${tests_SRCS})
target_link_libraries(keyringeditor 
	${QT_QTCORE_LIBRARY}
	${QT_QTGUI_LIBRARY}
	${QCA2_LIBRARIES}
	kpilot
	kpilot_conduit_base
)
