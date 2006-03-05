kde4_header()

add_subdirectory( doc ) 
add_subdirectory( scripts ) 
add_subdirectory( src ) 
add_subdirectory( zoneinfo ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#
#KDE_OPTIONS = foreign
#
#EXTRA_DIST = \
#	ChangeLog \
#	README \
#	TEST \
#	TODO
#
#SUBDIRS = doc scripts src zoneinfo
#
