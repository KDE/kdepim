kde4_header()

set(libresourcekolabshared_SRCS 
	${CMAKE_SOURCE_DIR}/kresources/kolab/shared/resourcekolabbase.cpp 
	${CMAKE_SOURCE_DIR}/kresources/kolab/shared/kmailconnection.cpp 
	${CMAKE_SOURCE_DIR}/kresources/kolab/shared/kolabbase.cpp
	${CMAKE_SOURCE_DIR}/kresources/kolab/shared/subresource.cpp 
	)
#kde4_add_dcop_skels(libresourcekolabshared_SRCS 
#	${CMAKE_SOURCE_DIR}/kresources/kolab/shared/kmailconnection.h
#	)

add_subdirectory( shared ) 
add_subdirectory( kabc ) 
add_subdirectory( knotes ) 
add_subdirectory( kcal ) 




########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = shared kabc knotes kcal
#
#messages: rc.cpp
#	$(XGETTEXT) */*.cpp -o $(podir)/kres_kolab.pot
