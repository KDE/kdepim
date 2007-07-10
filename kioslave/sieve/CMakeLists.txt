
include_directories( ${SASL2_INCLUDE_DIR}  )

configure_file (config-sieve.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-sieve.h )

########### next target ###############

set(kio_sieve_PART_SRCS sieve.cpp )


kde4_add_plugin(kio_sieve ${kio_sieve_PART_SRCS})



target_link_libraries(kio_sieve  ${KDE4_KIO_LIBS} ${SASL2_LIBRARIES})

install(TARGETS kio_sieve  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES sieve.protocol  DESTINATION ${SERVICES_INSTALL_DIR})
