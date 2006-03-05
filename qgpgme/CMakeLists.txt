kde4_header()

add_subdirectory( tests ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(qgpgme_LIB_SRCS eventloopinteractor.cpp dataprovider.cpp )

kde4_automoc(${qgpgme_LIB_SRCS})

kde4_add_library(qgpgme SHARED ${qgpgme_LIB_SRCS})

target_link_libraries(qgpgme  ${QT_AND_KDECORE_LIBS} )

set_target_properties(qgpgme PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} qgpgme )


########### install files ###############

install_files( /include/qgpgme FILES  )

kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = . tests
#
#INCLUDES =  -I$(top_srcdir) $(all_includes)
#
#lib_LTLIBRARIES = libqgpgme.la
#
#qgpgmedir = $(includedir)/qgpgme
#qgpgme_HEADERS = eventloopinteractor.h dataprovider.h
#
#libqgpgme_la_SOURCES = eventloopinteractor.cpp dataprovider.cpp
#libqgpgme_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined
#libqgpgme_la_LIBADD = $(LIB_QT) ../gpgmepp/libgpgme++.la
#
#METASOURCES = AUTO
#
#messages:
#	$(XGETTEXT) *.cpp *.h -o $(podir)/libqgpgme.pot
#
#include $(top_srcdir)/admin/Doxyfile.am
#
