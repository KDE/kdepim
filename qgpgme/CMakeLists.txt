
add_subdirectory( tests ) 




########### next target ###############

set(qgpgme_LIB_SRCS eventloopinteractor.cpp dataprovider.cpp )

kde4_automoc(${qgpgme_LIB_SRCS})

kde4_add_library(qgpgme SHARED ${qgpgme_LIB_SRCS})

#Laurent add test !
target_link_libraries(qgpgme  ${KDE4_KDECORE_LIBS}  gpgme++ )

set_target_properties(qgpgme PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS qgpgme  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

#install( FILES   DESTINATION ${INCLUDE_INSTALL_DIR}/qgpgme)




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
