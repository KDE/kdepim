kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

##	$Id: Makefile.am 331880 2004-07-22 21:51:23Z dfaure $	
#
#noinst_LTLIBRARIES = libversit.la
#
#libversit_la_SOURCES = vcc.c vobject.c
#
