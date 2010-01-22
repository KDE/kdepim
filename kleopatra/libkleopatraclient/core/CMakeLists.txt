if ( ASSUAN2_FOUND )
include_directories( ${ASSUAN2_INCLUDES} ${CMAKE_SOURCE_DIR}/kleopatra )
else ( ASSUAN2_FOUND )
include_directories( ${ASSUAN_INCLUDES} ${CMAKE_SOURCE_DIR}/kleopatra )
endif ( ASSUAN2_FOUND )

add_definitions( -D_ASSUAN_ONLY_GPG_ERRORS -DQT_NO_CAST_FROM_ASCII -DQT_NO_KEYWORDS -DQT_NO_CAST_TO_ASCII )

if ( WIN32 )
  set( _kleopatraclientcore_extra_SRCS
    ../../utils/gnupg-registry.c
    )
else ( WIN32 )
  set( _kleopatraclientcore_extra_SRCS )
endif ( WIN32 )

kde4_add_library( kleopatraclientcore SHARED
  ${_kleopatraclientcore_extra_SRCS}
  initialization.cpp
  command.cpp
  selectcertificatecommand.cpp
  signencryptfilescommand.cpp
  decryptverifyfilescommand.cpp
  )

set_target_properties( kleopatraclientcore
  PROPERTIES
    VERSION   ${libkleopatraclient_version}
    SOVERSION ${libkleopatraclient_soversion}
  )


if ( WIN32 )
if ( ASSUAN2_FOUND )
  target_link_libraries( kleopatraclientcore ${QT_QTCORE_LIBRARY} ${ASSUAN2_LIBRARIES} ws2_32 )
else ( ASSUAN2_FOUND )
  target_link_libraries( kleopatraclientcore ${QT_QTCORE_LIBRARY} ${ASSUAN_VANILLA_LIBRARIES} ws2_32 )
endif ( ASSUAN2_FOUND )
else ( WIN32 )
if ( ASSUAN2_FOUND )
  target_link_libraries( kleopatraclientcore ${QT_QTCORE_LIBRARY} ${ASSUAN2_LIBRARIES} )
else ( ASSUAN2_FOUND )
  target_link_libraries( kleopatraclientcore ${QT_QTCORE_LIBRARY} ${ASSUAN_PTHREAD_LIBRARIES} )
endif ( ASSUAN2_FOUND )
endif ( WIN32 )

install(
  TARGETS kleopatraclientcore
  DESTINATION ${LIB_INSTALL_DIR}
  )
  

