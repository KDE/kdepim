



########### next target ###############

set(kio_mbox_PART_SRCS 
   mbox.cc 
   mboxfile.cc 
   readmbox.cc 
   stat.cc 
   urlinfo.cc )


kde4_add_plugin(kio_mbox ${kio_mbox_PART_SRCS})



target_link_libraries(kio_mbox  ${KDE4_KIO_LIBS} )

install(TARGETS kio_mbox  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES mbox.protocol  DESTINATION ${SERVICES_INSTALL_DIR})


