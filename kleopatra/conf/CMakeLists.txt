
include_directories( ${CMAKE_SOURCE_DIR}/libkleo ${CMAKE_SOURCE_DIR}/libkleo/ui ${CMAKE_BINARY_DIR}/libkleo  )


########### next target ###############


########### next target ###############

set(kcm_kleopatra_PART_SRCS 
   dirservconfigpage.cpp 
   appearanceconfigpage.cpp 
   appearanceconfigwidget.cpp 
   dnorderconfigpage.cpp )

kde4_automoc(${kcm_kleopatra_PART_SRCS})

kde4_add_ui3_files(kcm_kleopatra_PART_SRCS appearanceconfigwidgetbase.ui )

kde4_add_plugin(kcm_kleopatra ${kcm_kleopatra_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kleopatra )

target_link_libraries(kcm_kleopatra  ${KDE4_KDEUI_LIBS} kleopatra )

install(TARGETS kcm_kleopatra  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kleopatra_config_dirserv.desktop  	kleopatra_config_appear.desktop  	kleopatra_config_dnorder.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

