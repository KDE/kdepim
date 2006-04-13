kde4_header()

include_directories( 
	${CMAKE_SOURCE_DIR}/kaddressbook/interfaces 
	${CMAKE_SOURCE_DIR}/kaddressbook/
	${KDE4_INCLUDE_DIR} 
	${QT_INCLUDES} 
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

kde4_add_kcfg_files(libkabcommon_SRCS ${CMAKE_SOURCE_DIR}/kaddressbook/common/kabprefs_base.kcfgc )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= -I$(top_srcdir)/kaddressbook/interfaces \
#          -I$(top_srcdir) \
#          $(all_includes)
#
#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libkabcommon.la
#libkabcommon_la_SOURCES = kabprefs.cpp kabprefs_base.kcfgc locationmap.cpp filter.cpp
#libkabcommon_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined
#libkabcommon_la_LIBADD = $(top_builddir)/kaddressbook/interfaces/libkabinterfaces.la \
#                         $(top_builddir)/libkdepim/libkdepim.la
#noinst_HEADERS = filter.h kabprefs.h
