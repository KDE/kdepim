
add_subdirectory( ktnef ) 
add_subdirectory( lib ) 
add_subdirectory( gui ) 




########### install files ###############





#original Makefile.am contents follow:

#SUBDIRS = ktnef lib gui
#
#messages: rc.cpp
#	$(EXTRACTRC) gui/*.rc gui/*.ui >> rc.cpp
#	$(XGETTEXT) rc.cpp gui/*.cpp lib/*.cpp -o $(podir)/ktnef.pot
