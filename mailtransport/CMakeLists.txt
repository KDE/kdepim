
add_subdirectory( kconf_update )
add_subdirectory( tests )

add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")

set(mailtransport_lib_srcs
  transport.cpp
  transportcombobox.cpp
  transportconfigdialog.cpp
  transportjob.cpp
  transportmanager.cpp
  transportmanagementwidget.cpp
  transporttypedialog.cpp
  sendmailjob.cpp
  smtpjob.cpp
  precommandjob.cpp
  legacydecrypt.cpp
)

kde4_automoc(${mailtransport_lib_srcs})

kde4_add_ui_files(mailtransport_lib_srcs
  smtpsettings.ui
  sendmailsettings.ui
  transportmanagementwidget.ui
  transporttypedialog.ui
)
kde4_add_kcfg_files(mailtransport_lib_srcs transportbase.kcfgc)

kde4_add_library(mailtransport SHARED ${mailtransport_lib_srcs})
target_link_libraries(mailtransport ${KDE4_KIO_LIBS} kwalletclient kdepim)
set_target_properties(mailtransport PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(TARGETS mailtransport DESTINATION ${LIB_INSTALL_DIR})
install(FILES mailtransport.kcfg  DESTINATION ${KCFG_INSTALL_DIR})


set(mailtransport_kcm_srcs configmodule.cpp)
kde4_automoc(${mailtransport_kcm_srcs})
kde4_add_plugin(kcm_mailtransport ${mailtransport_kcm_srcs})
target_link_libraries(kcm_mailtransport  ${KDE4_KDEUI_LIBS} mailtransport)

install(TARGETS kcm_mailtransport  DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_mailtransport.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
