kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${KDE4_ICON_DIR}/default.kde/32x32/devices FILES mobile_phone.png mobile_organizer.png mobile_camera.png  	mobile_musicplayer.png mobile_unknown.png  )

kde4_footer()



#original Makefile.am contents follow:

#libkmobilepics_DATA = \
#	mobile_phone.png mobile_organizer.png mobile_camera.png \
#	mobile_musicplayer.png mobile_unknown.png 
#
## libkmobilepicsdir = $(kde_datadir)/kmobile/pics
#libkmobilepicsdir = $(kde_icondir)/default.kde/32x32/devices
