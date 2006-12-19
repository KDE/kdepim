
add_subdirectory( kconf_update )
add_subdirectory( tests )

set(mailtransport_lib_srcs
  transport.cpp
  transportcombobox.cpp
  transportconfigdialog.cpp
  transportmanager.cpp
  transportmanagementwidget.cpp
  transporttypedialog.cpp
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

