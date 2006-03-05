kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kpilot/icons FILES  )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#kpalmdocicondir = $(kde_datadir)/kpilot/icons
#kpalmdocicon_ICON = kpalmdoc
#
#KDE_ICON = kpalmdoc
#
