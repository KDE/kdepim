set( libkleopatraclient_soversion 0.1.0 )
set( libkleopatraclient_version ${libkleopatraclient_soversion} )

add_subdirectory( core )
add_subdirectory( gui )

install(
  FILES kleopatraclient_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/libkleopatraclient
  )
