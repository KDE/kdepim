project(kleopatra)

include_directories( ${CMAKE_SOURCE_DIR}/libkleo ${Boost_INCLUDE_DIR} )
add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

set(libconf_SRCS  ${CMAKE_SOURCE_DIR}/kleopatra/conf/configuredialog.cpp )
add_subdirectory( conf ) 
add_subdirectory( kwatchgnupg ) 
add_subdirectory( tests )

########### next target ###############

set(kleopatra_bin_SRCS ${libconf_SRCS}
   models/keylistmodel.cpp
   customactions.cpp 
   aboutdata.cpp 
   main.cpp 
   certmanager.cpp 
   hierarchyanalyser.cpp 
   certificatewizardimpl.cpp 
   certificateinfowidgetimpl.cpp 
   crlview.cpp 
   certlistview.cpp )


kde4_add_ui_files(kleopatra_bin_SRCS certificateinfowidget.ui certificatewizard.ui)

kde4_add_executable(kleopatra_bin ${kleopatra_bin_SRCS})
set_target_properties(kleopatra_bin  PROPERTIES OUTPUT_NAME kleopatra)

target_link_libraries(kleopatra_bin ${KDE4_KUTILS_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KABC_LIBS} kleo ${KDE4_GPGMEPP_LIBS} ${KDE4_QGPGME_LIBS} ${GPGME_LIBRARIES})

install(TARGETS kleopatra_bin  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kleopatra_import.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kleopatraui.rc  DESTINATION ${DATA_INSTALL_DIR}/kleopatra)


