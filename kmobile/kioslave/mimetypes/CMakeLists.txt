kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${MIME_INSTALL_DIR}/inode FILES mobile_addressbook.desktop mobile_calendar.desktop mobile_notes.desktop  	mobile_device.desktop )

kde4_footer()



#original Makefile.am contents follow:

#mimetype_DATA = \
#	mobile_addressbook.desktop mobile_calendar.desktop mobile_notes.desktop \
#	mobile_device.desktop
#mimetypedir = $(kde_mimedir)/inode
