project( ksendemail )

if(KMAIL_SUPPORTED)
  add_definitions( -DQT_NO_CAST_FROM_ASCII )
  add_definitions( -DQT_NO_CAST_TO_ASCII )

  set( ksendemail_SRCS main.cpp mailerservice.cpp )

  qt4_add_dbus_interfaces( ksendemail_SRCS  ${CMAKE_BINARY_DIR}/kmail/org.kde.kmail.kmail.xml )

  kde4_add_executable( ksendemail ${ksendemail_SRCS})
  add_dependencies( ksendemail kmail_xml )
  target_link_libraries( ksendemail ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} )

  install( TARGETS ksendemail ${INSTALL_TARGETS_DEFAULT_ARGS} )

else()
  message(STATUS "Ksendemail will NOT be built.")
endif()
