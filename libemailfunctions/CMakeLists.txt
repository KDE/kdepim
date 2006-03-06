kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

kde4_add_dcop_skels(libemailfunctions_SRCS ${CMAKE_SOURCE_DIR}/libemailfunctions/networkstatus.h )

########### next target ###############


########### install files ###############

install_files( /include/libemailfunctions FILES idmapper.h )

kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
#noinst_LTLIBRARIES = libemailfunctions.la
#libemailfunctions_la_SOURCES = email.cpp idmapper.cpp kasciistricmp.cpp \
#	kasciistringtools.cpp \
#	networkstatus.cpp networkstatus.skel
#libemailfunctions_la_LDFLAGS = $(all_libraries) -no-undefined
### Since this is a noinst library, in case of --enable-closure we need
### to link it explicitly to the libraries that it uses.
#libemailfunctions_la_LIBADD = $(LIB_QT) $(LIB_KDECORE)
#
#emailfunctionsincludedir = $(includedir)/libemailfunctions
#emailfunctionsinclude_HEADERS = idmapper.h
#
#METASOURCES = AUTO
#
## No messages target. Those files are part of libkdepim.pot.
#
#include $(top_srcdir)/admin/Doxyfile.am
