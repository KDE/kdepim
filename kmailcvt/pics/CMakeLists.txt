kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kmailcvt/pics FILES step1.png )

kde4_footer()



#original Makefile.am contents follow:

#pics_DATA = step1.png
#picsdir = $(kde_datadir)/kmailcvt/pics
#
