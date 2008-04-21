include_directories( ${CMAKE_SOURCE_DIR}/kleopatra ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

if ( ONLY_KLEO )
  add_definitions( -DONLY_KLEO )
  set( _kcm_kleopatra_kleo_only_SRCS
    ../utils/kleo_kicondialog.cpp
    )
else ( ONLY_KLEO )
  set( _kcm_kleopatra_extra_libs ${KDE4_KIO_LIBS} )
endif ( ONLY_KLEO )

set(kcm_kleopatra_PART_SRCS 
   ${_kcm_kleopatra_kleo_only_SRCS}
   dirservconfigpage.cpp 
   appearanceconfigpage.cpp 
   appearanceconfigwidget.cpp 
   dnorderconfigpage.cpp
   smimevalidationconfigurationwidget.cpp
   smimevalidationconfigurationpage.cpp
   )

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=5152 )

kde4_add_ui_files( kcm_kleopatra_PART_SRCS
  appearanceconfigwidget.ui
  smimevalidationconfigurationwidget.ui
  )

kde4_add_kcfg_files( kcm_kleopatra_PART_SRCS ${CMAKE_SOURCE_DIR}/kleopatra/common/kleopatraprefs.kcfgc )
kde4_add_plugin(kcm_kleopatra ${kcm_kleopatra_PART_SRCS})



target_link_libraries(kcm_kleopatra ${KDE4_KDEUI_LIBS} kleo )

install(TARGETS kcm_kleopatra  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES
  kleopatra_config_dirserv.desktop
  kleopatra_config_appear.desktop
  kleopatra_config_dnorder.desktop
  kleopatra_config_smimevalidation.desktop
  DESTINATION ${SERVICES_INSTALL_DIR})

