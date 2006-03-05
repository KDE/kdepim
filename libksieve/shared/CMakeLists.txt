kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libksieve ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/libksieve $(all_includes)
#
#noinst_LTLIBRARIES = libksieve_shared.la
#
#libksieve_shared_la_SOURCES = error.cpp
#libksieve_shared_la_LIBADD = $(LIB_KDECORE)
#libksieve_shared_la_LDFLAGS = $(all_libraries) -no-undefined
#
