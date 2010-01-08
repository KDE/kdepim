include_directories(
  ${CMAKE_BINARY_DIR}/ontologies
  ${CMAKE_SOURCE_DIR}/libkleo
  ${Boost_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/shared
  ${CMAKE_BINARY_DIR}/messageviewer
  ${SOPRANO_INCLUDE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )
add_definitions(-DNEPOMUK_RESOURCE_STATIC)

set(akonadi_nepomuk_email_feeder_SRCS
  nepomukemailfeeder.cpp
  task.cpp
  messageanalyzer.cpp
  messagesearch.cpp
  configdialog.cpp
  shared/nepomukfeederagentbase.cpp
  shared/sparqlbuilder.cpp
  shared/selectsparqlbuilder.cpp
)

kde4_add_app_icon(akonadi_nepomuk_email_feeder_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/nepomuk.png")
kde4_add_kcfg_files(akonadi_nepomuk_email_feeder_SRCS settings.kcfgc)
kde4_add_ui_files(akonadi_nepomuk_email_feeder_SRCS configdialog.ui)

kde4_add_executable(akonadi_nepomuk_email_feeder ${akonadi_nepomuk_email_feeder_SRCS})

target_link_libraries(akonadi_nepomuk_email_feeder
  niefast_apps
  messagecore
  messageviewer
  ${SOPRANO_LIBRARIES}
  ${SOPRANO_CLIENT_LIBRARIES}
  ${KDE4_KDEUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${QT_QT3SUPPORT_LIBRARY}
  ${NEPOMUK_LIBRARIES}
  ${STRIGI_STREAMS_LIBRARY}
  ${STRIGI_STREAMANALYZER_LIBRARY}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${QGPGME_LIBRARIES}
)

install(TARGETS akonadi_nepomuk_email_feeder ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES nepomukemailfeeder.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")

