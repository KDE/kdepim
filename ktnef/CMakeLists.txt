kde4_header()

add_subdirectory( ktnef ) 
add_subdirectory( lib ) 
add_subdirectory( gui ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = ktnef lib gui
#
#messages: rc.cpp
#	$(EXTRACTRC) gui/*.rc gui/*.ui >> rc.cpp
#	$(XGETTEXT) rc.cpp gui/*.cpp lib/*.cpp -o $(podir)/ktnef.pot
