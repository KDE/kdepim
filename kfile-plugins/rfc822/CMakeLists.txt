



########### next target ###############

set(kfile_rfc822_PART_SRCS kfile_rfc822.cpp )

kde4_automoc(${kfile_rfc822_PART_SRCS})

kde4_add_plugin(kfile_rfc822 ${kfile_rfc822_PART_SRCS})



target_link_libraries(kfile_rfc822  ${KDE4_KIO_LIBS} )

install(TARGETS kfile_rfc822  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kfile_rfc822.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


