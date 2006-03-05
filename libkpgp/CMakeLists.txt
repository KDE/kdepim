kde4_header()

add_subdirectory( pics ) 

include_directories(${CMAKE_SOURCE_DIR}/libkdepim  ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kpgp_LIB_SRCS 
   kpgpblock.cpp 
   kpgpui.cpp 
   kpgpkey.cpp 
   kpgp.cpp 
   kpgpbase.cpp 
   kpgpbaseG.cpp 
   kpgpbase2.cpp 
   kpgpbase5.cpp 
   kpgpbase6.cpp )

kde4_automoc(${kpgp_LIB_SRCS})

kde4_add_library(kpgp SHARED ${kpgp_LIB_SRCS})

target_link_libraries(kpgp  ${LIB_KDE3SUPPORT} )

set_target_properties(kpgp PROPERTIES VERSION 2.2.0 SOVERSION 2 )
install_targets(${LIB_INSTALL_DIR} kpgp )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kconf_update FILES kpgp.upd )

kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#SUBDIRS = pics
#
#lib_LTLIBRARIES = libkpgp.la
#
#libkpgp_la_SOURCES = \
#			  kpgpblock.cpp \
#                          kpgpui.cpp \
#                          kpgpkey.cpp \
#                          kpgp.cpp \
#                          kpgpbase.cpp \
#                          kpgpbaseG.cpp \
#                          kpgpbase2.cpp \
#                          kpgpbase5.cpp \
#                          kpgpbase6.cpp
#
#libkpgp_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 4:0:2
#
#libkpgp_la_LIBADD = $(LIB_KDEUI) $(LIB_POLL)
#
#METASOURCES = AUTO
#
#updatedir = $(kde_datadir)/kconf_update
#update_DATA = kpgp.upd
#update_SCRIPTS = kpgp-3.1-upgrade-address-data.pl
#
#messages: 
#	$(XGETTEXT) *.cpp *.h -o $(podir)/libkpgp.pot
#
#include $(top_srcdir)/admin/Doxyfile.am
