
include_directories( ${SASL2_INCLUDE_DIR} )

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

########### next target ###############

set(kio_imap4_PART_SRCS 
   imapcommand.cc 
   imaplist.cc 
   mailaddress.cc 
   mimeheader.cc 
   imap4.cc 
   imapinfo.cc 
   imapparser.cc 
   mailheader.cc 
   mimehdrline.cc 
   mimeio.cc )


kde4_add_plugin(kio_imap4 ${kio_imap4_PART_SRCS})

target_link_libraries(kio_imap4  ${KDE4_KDE3SUPPORT_LIBS}  ${SASL2_LIBRARIES} ${KDE4_KMIME_LIBS} ${KDE4_KIMAP_LIBS})

install(TARGETS kio_imap4  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES imap4.protocol imaps.protocol  DESTINATION ${SERVICES_INSTALL_DIR})
