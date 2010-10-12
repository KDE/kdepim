include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(conduit_abbrowser_SRCS
	resolutionDialog.cc
	abbrowser-factory.cc
	abbrowser-setup.cc
	kabcRecord.cc
	abbrowser-conduit.cc
)

set(conduit_abbrowser_UIS
	resolutionDialog_base.ui
	kaddressbookConduit.ui
)

set(conduit_abbrowser_KCFGS
	abbrowserSettings.kcfgc
)

kde3_add_kcfg_files(conduit_abbrowser_SRCS ${conduit_abbrowser_KCFGS})
kde3_add_ui_files(conduit_abbrowser_SRCS ${conduit_abbrowser_UIS})
kde3_automoc(${conduit_abbrowser_SRCS})
add_library(conduit_address SHARED ${conduit_abbrowser_SRCS})
target_link_libraries(conduit_address kabc_file kabc)
set_target_properties(conduit_address PROPERTIES LOCATION ${KDE3_PLUGIN_INSTALL_DIR}
	INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib
	PREFIX ""
)

kde3_install_libtool_file(conduit_address)

install(
	TARGETS conduit_address
	LIBRARY DESTINATION ${KDE3_PLUGIN_INSTALL_DIR}
)

install(
	FILES abbrowser_conduit.desktop DESTINATION ${KDE3_SERVICES_DIR}
)

install(
	FILES abbrowserconduit.kcfg DESTINATION ${KDE3_KCFG_DIR}
)

