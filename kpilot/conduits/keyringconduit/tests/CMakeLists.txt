include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../
	${CMAKE_CURRENT_BINARY_DIR}
)

set(tests_SRCS
	keyringlistmodel.cc
	keyringviewer.cc
	main.cc
	../pi-keyring.cc
	../keyringconduit.cc
	../keyringhhdataproxy.cc
	../keyringhhrecord.cc
)

kde4_add_kcfg_files(tests_SRCS ../keyringsettings.kcfgc)
kde4_add_ui_files(tests_SRCS viewer.ui)
kde4_add_executable(testje ${tests_SRCS})
target_link_libraries(testje ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} kpilot kpilot_conduit_base ${QCA2_LIBRARIES})
