kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kaddressbook/icons FILES  )
install_files( ${DATA_INSTALL_DIR}/kaddressbook/pics FILES world.jpg )

kde4_footer()



#original Makefile.am contents follow:

#kaddressbookicondir = $(kde_datadir)/kaddressbook/icons
#kaddressbookicon_ICON = AUTO
#
#kabpics_DATA = world.jpg
#kabpicsdir = $(kde_datadir)/kaddressbook/pics
