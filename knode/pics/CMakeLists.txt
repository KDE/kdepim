kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/knode/pics FILES stat_edit.png mail.png stat_saved.png  	ctlart.png greyball.png ignore.png  	newsubs.png stat_sent.png  	eyes.png greyballchk.png snderr.png  	group.png posting.png stat_cncl.png          pgp-keys.png group_big.png article.png )
install_files( ${DATA_INSTALL_DIR}/knode/icons FILES  )

kde4_footer()



#original Makefile.am contents follow:

#pics_DATA = stat_edit.png mail.png stat_saved.png \
#	ctlart.png greyball.png ignore.png \
#	newsubs.png stat_sent.png \
#	eyes.png greyballchk.png snderr.png \
#	group.png posting.png stat_cncl.png \
#        pgp-keys.png group_big.png article.png
#
#picsdir = $(kde_datadir)/knode/pics
#
#EXTRA_DIST = $(pics_DATA)
#knodeicondir = $(kde_datadir)/knode/icons
#knodeicon_ICON = message_reply mail_get_all
