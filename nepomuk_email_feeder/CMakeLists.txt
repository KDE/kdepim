project(akonadi_nepomuk_mail_feeder)

include_directories(
  ${CMAKE_BINARY_DIR}/ontologies
  ${CMAKE_SOURCE_DIR}/libkleo
  ${Boost_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/shared
  ${CMAKE_BINARY_DIR}/messageviewer
  ${SOPRANO_INCLUDE_DIR}
  ${STRIGI_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/shared/plugin
  ${CMAKE_CURRENT_SOURCE_DIR}/shared/ontologies
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

set(akonadi_nepomuk_email_feeder_SRCS
  nepomukmailfeeder.cpp
)

kde4_add_plugin( akonadi_nepomuk_mail_feeder ${akonadi_nepomuk_email_feeder_SRCS} )
target_link_libraries(akonadi_nepomuk_mail_feeder
  messagecore
  messageviewer
  ${SOPRANO_LIBRARIES}
  ${SOPRANO_CLIENT_LIBRARIES}
  ${KDE4_KDEUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${QT_QTSCRIPT_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${NEPOMUK_LIBRARIES}
  ${NEPOMUK_QUERY_LIBRARIES}
  ${STRIGI_STREAMS_LIBRARY}
  ${STRIGI_STREAMANALYZER_LIBRARY}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${QGPGME_LIBRARIES}
  ${KDE4_KIDLETIME_LIBRARY}
)

########### install files ###############

install(
  FILES nepomukmailfeeder.desktop
  DESTINATION ${SERVICES_INSTALL_DIR})

install(
  TARGETS akonadi_nepomuk_mail_feeder
  DESTINATION ${PLUGIN_INSTALL_DIR})

