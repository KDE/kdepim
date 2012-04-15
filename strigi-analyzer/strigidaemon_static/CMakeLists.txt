find_package(LibXml2 REQUIRED)
find_package(Iconv REQUIRED)
find_package(LibDBus REQUIRED)
find_package(CLucene REQUIRED)

add_executable(strigidaemon dummy.cpp)

target_link_libraries(strigidaemon \
  libstrigidaemon
  ${STRIGI_DAEMONSTATIC_LIBRARY}
  ${WCECOMPAT_LIBRARIES}
  ${STRIGI_STREAMANALYZER_LIBRARY}
  ${STRIGI_STREAMS_LIBRARY}
  ${STRIGI_STRIGIQTDBUSCLIENT_LIBRARY}
  ${STRIGI_XESAM_LIBRARY}
  ${STRIGI_EVENTLISTENER_LIBRARY}
  ${STRIGI_COMBINEDINDEX_LIBRARY}
  ${DBUS_LIBRARIES}
  ${STRIGI_DBUSSERVER_LIBRARY}
  ${STRIGI_STRIGIDAEMONCONFIGURATION_LIBRARY}
  ${STRIGI_METADATAPROPERTIES_LIBRARY}
  ${ICONV_LIBRARIES}
  ${LIBXML2_LIBRARIES}
  ${STRIGI_XMLSTREAM_LIBRARY}
  ${STRIGI_DBUSCPP_LIBRARY}
  ${STRIGI_CLUCENE_LIBRARY}
  ${STRIGI_PLUGIN_TXT_LIBRARY}
  ${STRIGI_PLUGIN_CPP_LIBRARY}
  ${CLUCENE_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ws2
  ctg
  ics
  mail
  vcf
  messageviewer_bodypartformatter_text_calendar
  akonadi_serializer_mail
  akonadi_serializer_addressee
  akonadi_serializer_contactgroup
  akonadi_serializer_kcalcore
  incidenceeditorsngmobile
)

install(TARGETS strigidaemon RUNTIME DESTINATION bin)

