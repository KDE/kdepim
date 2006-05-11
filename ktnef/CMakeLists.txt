kde4_header()

add_subdirectory( ktnef ) 
add_subdirectory( lib ) 
add_subdirectory( gui ) 




########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = ktnef lib gui
#
#messages: rc.cpp
#	$(EXTRACTRC) gui/*.rc gui/*.ui >> rc.cpp
#	$(XGETTEXT) rc.cpp gui/*.cpp lib/*.cpp -o $(podir)/ktnef.pot
