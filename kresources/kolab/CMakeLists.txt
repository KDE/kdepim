kde4_header()

add_subdirectory( shared ) 
add_subdirectory( kabc ) 
add_subdirectory( knotes ) 
add_subdirectory( kcal ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = shared kabc knotes kcal
#
#messages: rc.cpp
#	$(XGETTEXT) */*.cpp -o $(podir)/kres_kolab.pot
