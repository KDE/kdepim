# FIXME: apps cannot have a compile time dependency on kdepim/runtime!
include_directories(
  ${kdepim-runtime_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/nepomuk_email_feeder/shared
  ${CMAKE_BINARY_DIR}/ontologies
  ${Boost_INCLUDE_DIR}
  )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )
add_definitions(-DNEPOMUK_RESOURCE_STATIC)

set(akonadi_nepomuk_rss_feeder_SRCS
  nepomukrssfeeder.cpp
  ${CMAKE_SOURCE_DIR}/nepomuk_email_feeder/shared/nepomukfeederagentbase.cpp
  ${CMAKE_SOURCE_DIR}/nepomuk_email_feeder/shared/sparqlbuilder.cpp
  ${CMAKE_SOURCE_DIR}/nepomuk_email_feeder/shared/selectsparqlbuilder.cpp
)

kde4_add_app_icon(akonadi_nepomuk_rss_feeder_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/nepomuk.png")
kde4_add_executable(akonadi_nepomuk_rss_feeder ${akonadi_nepomuk_rss_feeder_SRCS})

target_link_libraries(akonadi_nepomuk_rss_feeder
  niefast_apps
  krss
  ${KDE4_KDEUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${NEPOMUK_LIBRARIES}
  ${STRIGI_STREAMS_LIBRARY}
  ${STRIGI_STREAMANALYZER_LIBRARY}
  ${SOPRANO_LIBRARIES}
)

install(TARGETS akonadi_nepomuk_rss_feeder ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES nepomukrssfeeder.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")
