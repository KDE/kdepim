set( libkleopatraclient_soversion 0.2.0 )
set( libkleopatraclient_version ${libkleopatraclient_soversion} )

add_subdirectory( core )
add_subdirectory( gui )

add_subdirectory( tests )

install(
  FILES kleopatraclient_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/libkleopatraclient
  )
