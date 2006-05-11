kde4_header()

add_subdirectory( doc ) 
add_subdirectory( scripts ) 
add_subdirectory( src ) 
add_subdirectory( zoneinfo ) 




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
