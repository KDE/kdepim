kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/libkleopatra/pics FILES key.png key_ok.png key_bad.png key_unknown.png chiasmus_chi.png )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



#original Makefile.am contents follow:

#picsdir = $(kde_datadir)/libkleopatra/pics
#pics_DATA = key.png key_ok.png key_bad.png key_unknown.png chiasmus_chi.png
#
#KDE_ICON = gpg gpgsm
