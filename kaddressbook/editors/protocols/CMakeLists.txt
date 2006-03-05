kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kaddressbook FILES icqprotocol.desktop aimprotocol.desktop ircprotocol.desktop  			   smsprotocol.desktop gaduprotocol.desktop  			   jabberprotocol.desktop yahooprotocol.desktop  			   msnprotocol.desktop groupwiseprotocol.desktop  			   meanwhileprotocol.desktop skypeprotocol.desktop )

kde4_footer()



#original Makefile.am contents follow:

#servicedir = $(kde_servicesdir)/kaddressbook
#service_DATA = icqprotocol.desktop aimprotocol.desktop ircprotocol.desktop \
#			   smsprotocol.desktop gaduprotocol.desktop \
#			   jabberprotocol.desktop yahooprotocol.desktop \
#			   msnprotocol.desktop groupwiseprotocol.desktop \
#			   meanwhileprotocol.desktop skypeprotocol.desktop
