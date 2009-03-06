project( ksendemail )

if(KMAIL_SUPPORTED)

  find_package( KDE4 REQUIRED )
  include_directories( ${CMAKE_SOURCE_DIR} ${KDE4_INCLUDES} ${QT_QTDBUS_INCLUDE_DIR} )

  set( ksendemail_SRCS main.cpp mailerservice.cpp )

  qt4_add_dbus_interfaces( ksendemail_SRCS  ${CMAKE_BINARY_DIR}/kmail/org.kde.kmail.kmail.xml )

  kde4_add_executable( ksendemail ${ksendemail_SRCS})
  target_link_libraries( ksendemail ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} )

  install( TARGETS ksendemail ${INSTALL_TARGETS_DEFAULT_ARGS} )

else(KMAIL_SUPPORTED)
  message(STATUS "Ksendemail will NOT be built.")
endif(KMAIL_SUPPORTED)
