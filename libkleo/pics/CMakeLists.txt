



########### install files ###############

install( FILES key.png key_ok.png key_bad.png key_unknown.png chiasmus_chi.png  DESTINATION ${DATA_INSTALL_DIR}/libkleopatra/pics)

kde4_install_icons( ${ICON_INSTALL_DIR}  crystalsvg )




#original Makefile.am contents follow:

#picsdir = $(kde_datadir)/libkleopatra/pics
#pics_DATA = key.png key_ok.png key_bad.png key_unknown.png chiasmus_chi.png
#
#KDE_ICON = gpg gpgsm
