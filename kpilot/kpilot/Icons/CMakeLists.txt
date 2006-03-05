kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kpilot/icons FILES  )
install_files( ${DATA_INSTALL_DIR}/kpilot/pics FILES kpilot-splash.png )
install_files( ${DATA_INSTALL_DIR}/kpilot/icons FILES  )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#kpilotDaemonicondir = $(kde_datadir)/kpilot/icons
#kpilotDaemonicon_ICON = hotsync busysync nosync
#
#kpiloticondir = $(kde_datadir)/kpilot/icons
#kpiloticon_ICON = backup fastsync fullsync listsync restore \
#	kpilotaddress \
#	kpilotknotes \
#	kpilotfileinstaller \
#	kpilotdb \
#	kpilotcalendar \
#	kpilottodo \
#	kpilotbhotsync
#
#KDE_ICON = kpilot kpilotDaemon
#
#kpilotpicsdir = $(kde_datadir)/kpilot/pics
#kpilotpics_DATA = kpilot-splash.png
