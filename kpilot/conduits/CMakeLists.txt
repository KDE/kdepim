kde4_header()

add_subdirectory( docconduit ) 
add_subdirectory( knotes ) 
add_subdirectory( popmail ) 
add_subdirectory( timeconduit ) 
add_subdirectory( sysinfoconduit ) 
add_subdirectory( memofileconduit ) 

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(MAL_SUBDIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(ABC_SUBDIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(NOTEPAD_SUBDIR)")
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

####
#### The todo conduit is deprecated -- well, it has moved to
#### vcalconduit where it shares most of the code with the vcalconduit.
####
#### The NULL conduit is a neat programming example, but shouldn't be
#### installed on user systems.
####
#
#if include_malconduit
#MAL_SUBDIR = malconduit
#endif
#
#if include_abc
#ABC_SUBDIR = abbrowserconduit vcalconduit
#endif
#
#if include_perl
#PERL_SUBDIR = perlconduit
#endif
#
#if include_python
#PYTHON_SUBDIR = pythonconduit
#endif
#
#if include_notepad
#NOTEPAD_SUBDIR = notepadconduit
#endif
#
#SUBDIRS = \
#          docconduit \
#          knotes \
#          popmail \
#          timeconduit \
#          sysinfoconduit \
#	  memofileconduit \
#          $(MAL_SUBDIR) $(ABC_SUBDIR) $(NOTEPAD_SUBDIR)
#
####
#### Subdirs you might have for experimental purposes:
####
####	null             - a conduit that just logs a single message.
####	$(PERL_SUBDIR)   - fires off a perl interpreter in a thread.
####	$(PYTHON_SUBDIR) - starts a python interpreter in a thread.
####	notepadconduit   - a conduit to save notepad drawings as pngs.
####
#
#
