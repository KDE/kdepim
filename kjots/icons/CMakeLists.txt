kde4_header()




########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kjots/pics FILES filedel.png )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



#original Makefile.am contents follow:

#
#pics_DATA = filedel.png
#picsdir = $(kde_datadir)/kjots/pics
#
#KDE_ICON = kjots
#
