add_subdirectory( core )
add_subdirectory( gui )

install(
  FILES kleopatraclient_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/libkleopatraclient
  )
