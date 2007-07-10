
include_directories( ${CMAKE_SOURCE_DIR}/kresources/opengroupware/soap  )


########### next target ###############

set(kio_opengroupware_PART_SRCS opengroupware.cpp webdavhandler.cpp )


kde4_add_plugin(kio_opengroupware ${kio_opengroupware_PART_SRCS})



target_link_libraries(kio_opengroupware  ${KDE4_KDEIO_LIBS} ${KDE4_KCAL_LIBS} kdepim )

install(TARGETS kio_opengroupware  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES opengroupware.protocol opengroupwares.protocol  DESTINATION ${SERVICES_INSTALL_DIR})

