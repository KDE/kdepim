set( libkleopatraclient_version "0.2.1" )
set( libkleopatraclient_soversion "0" )

add_subdirectory( core )
add_subdirectory( gui )

add_subdirectory( tests )

install(
  FILES kleopatraclient_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/libkleopatraclient
  )
