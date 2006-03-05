kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/knode/filters FILES 1.fltr 2.fltr 3.fltr 4.fltr 5.fltr 6.fltr 7.fltr 8.fltr filters.rc )

kde4_footer()



#original Makefile.am contents follow:

#
#appsdir = $(kde_datadir)/knode/filters
#apps_DATA = 1.fltr 2.fltr 3.fltr 4.fltr 5.fltr 6.fltr 7.fltr 8.fltr filters.rc
#
