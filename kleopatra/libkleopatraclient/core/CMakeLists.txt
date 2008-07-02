include_directories( ${ASSUAN_INCLUDES} ${CMAKE_SOURCE_DIR}/kleopatra )

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
  #connection.cpp
  )

set_target_properties( kleopatraclientcore
  PROPERTIES
    VERSION   ${libkleopatraclient_version}
    SOVERSION ${libkleopatraclient_soversion}
  )


if ( WIN32 )
  target_link_libraries( kleopatraclientcore ${QT_QTCORE_LIBRARY} ${ASSUAN_VANILLA_LIBRARIES} Ws2_32 )
else ( WIN32 )
  target_link_libraries( kleopatraclientcore ${QT_QTCORE_LIBRARY} ${ASSUAN_PTHREAD_LIBRARIES} )
endif ( WIN32 )

install(
  TARGETS kleopatraclientcore
  DESTINATION ${LIB_INSTALL_DIR}
  )
  

install(
  FILES
    initialization.h
    command.h
    selectcertificatecommand.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/libkleopatraclient/core
  )

