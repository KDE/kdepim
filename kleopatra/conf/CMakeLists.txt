

########### next target ###############


########### next target ###############

set(kcm_kleopatra_PART_SRCS 
   dirservconfigpage.cpp 
   appearanceconfigpage.cpp 
   appearanceconfigwidget.cpp 
   dnorderconfigpage.cpp )

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=5152 )

kde4_add_ui_files(kcm_kleopatra_PART_SRCS appearanceconfigwidgetbase.ui )

kde4_add_plugin(kcm_kleopatra ${kcm_kleopatra_PART_SRCS})



target_link_libraries(kcm_kleopatra ${KDE4_KDEUI_LIBS} kleo ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS kcm_kleopatra  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kleopatra_config_dirserv.desktop  	kleopatra_config_appear.desktop  	kleopatra_config_dnorder.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

