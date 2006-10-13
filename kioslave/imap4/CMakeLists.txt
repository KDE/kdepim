
include_directories( ${CMAKE_SOURCE_DIR}/libkmime ${SASL2_INCLUDE_DIR} )


########### next target ###############

set(kio_imap4_PART_SRCS 
   imapcommand.cc 
   imaplist.cc 
   mailaddress.cc 
   mimeheader.cc 
   rfcdecoder.cc 
   imap4.cc 
   imapinfo.cc 
   imapparser.cc 
   mailheader.cc 
   mimehdrline.cc 
   mimeio.cc )

kde4_automoc(kio_imap4 ${kio_imap4_PART_SRCS})

kde4_add_plugin(kio_imap4 ${kio_imap4_PART_SRCS})

target_link_libraries(kio_imap4  ${KDE4_KDE3SUPPORT_LIBS}  ${SASL2_LIBRARIES} kmime  )

install(TARGETS kio_imap4  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES imap4.protocol imaps.protocol  DESTINATION ${SERVICES_INSTALL_DIR})
