kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( /include/gpgme++/interfaces FILES  )

kde4_footer()



#original Makefile.am contents follow:

#gpgmeppinterfacesdir = $(includedir)/gpgme++/interfaces
#gpgmeppinterfaces_HEADERS = editinteractor.h \
#			passphraseprovider.h progressprovider.h
