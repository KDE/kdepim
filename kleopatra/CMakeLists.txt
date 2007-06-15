project(kleopatra)

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

set(libconf_SRCS  ${CMAKE_SOURCE_DIR}/kleopatra/conf/configuredialog.cpp )
add_subdirectory( conf ) 
add_subdirectory( kwatchgnupg ) 

include_directories( ${CMAKE_SOURCE_DIR}/libkleo ${CMAKE_SOURCE_DIR}/gpgmepp/ )

########### next target ###############

set(kleopatra_bin_SRCS ${libconf_SRCS}
	customactions.cpp 
   aboutdata.cpp 
   main.cpp 
   certmanager.cpp 
   hierarchyanalyser.cpp 
   certificatewizardimpl.cpp 
   certificateinfowidgetimpl.cpp 
   crlview.cpp 
   certlistview.cpp )

kde4_automoc(${kleopatra_bin_SRCS})

kde4_add_ui_files(kleopatra_bin_SRCS certificateinfowidget.ui certificatewizard.ui)

kde4_add_executable(kleopatra_bin ${kleopatra_bin_SRCS})
set_target_properties(kleopatra_bin  PROPERTIES OUTPUT_NAME kleopatra)

target_link_libraries(kleopatra_bin ${KDE4_UTILS_LIBS} ${KDE4_KABC_LIBS} ${KDE4_KUTILS_LIBS} ${KDE4_KDE3SUPPORT_LIBS} kleopatra)

install(TARGETS kleopatra_bin  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kleopatra_import.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES kleopatraui.rc  DESTINATION ${DATA_INSTALL_DIR}/kleopatra)


