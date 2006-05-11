kde4_header()




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
