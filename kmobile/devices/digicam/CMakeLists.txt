kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES libkmobile_digicam.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kde_services_DATA = libkmobile_digicam.desktop
