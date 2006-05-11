kde4_header()




########### install files ###############

#install_files( ${DATA_INSTALL_DIR}/knode/pics FILES $(kmaildata_DATA) )
install_files( ${DATA_INSTALL_DIR}/kmail/pics FILES key_ok.png key_bad.png key_unknown.png key.png )

kde4_footer()



#original Makefile.am contents follow:

#
#kmaildata_DATA = key_ok.png key_bad.png key_unknown.png key.png
#knodedata_DATA = $(kmaildata_DATA)
#
#kmaildatadir = $(kde_datadir)/kmail/pics
#knodedatadir = $(kde_datadir)/knode/pics
#
#EXTRA_DIST = $(kmaildata_DATA)
#
