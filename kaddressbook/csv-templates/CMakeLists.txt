kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kaddressbook/csv-templates FILES kaddressbook.desktop outlook2000.desktop yahoo.desktop )

kde4_footer()



#original Makefile.am contents follow:

#templatedir = $(kde_datadir)/kaddressbook/csv-templates
#template_DATA = kaddressbook.desktop outlook2000.desktop yahoo.desktop
