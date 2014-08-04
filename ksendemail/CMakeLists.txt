project( ksendemail )

if(KMAIL_SUPPORTED)
  add_definitions( -DQT_NO_CAST_FROM_ASCII )
  add_definitions( -DQT_NO_CAST_TO_ASCII )

  set( ksendemail_SRCS main.cpp mailerservice.cpp )

  qt5_add_dbus_interfaces( ksendemail_SRCS  ${CMAKE_BINARY_DIR}/kmail/org.kde.kmail.kmail.xml )

  add_executable( ksendemail ${ksendemail_SRCS})
  add_dependencies( ksendemail kmail_xml )
  target_link_libraries( ksendemail   KF5::KIOCore Qt5::Widgets KF5::I18n KF5::WidgetsAddons Qt5::DBus)

  install( TARGETS ksendemail ${INSTALL_TARGETS_DEFAULT_ARGS} )

else()
  message(STATUS "Ksendemail will NOT be built.")
endif()
