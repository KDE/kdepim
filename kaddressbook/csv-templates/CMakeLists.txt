kde4_header()




########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kaddressbook/csv-templates FILES kaddressbook.desktop outlook2000.desktop yahoo.desktop )

kde4_footer()



#original Makefile.am contents follow:

#templatedir = $(kde_datadir)/kaddressbook/csv-templates
#template_DATA = kaddressbook.desktop outlook2000.desktop yahoo.desktop
