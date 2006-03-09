kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/ktnef ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(ktnef_LIB_SRCS 
   ktnefparser.cpp 
   ktnefproperty.cpp 
   ktnefattach.cpp 
   mapi.cpp 
   ktnefpropertyset.cpp 
   ktnefmessage.cpp 
   ktnefwriter.cpp 
   lzfu.cpp )

kde4_automoc(${ktnef_LIB_SRCS})

kde4_add_library(ktnef SHARED ${ktnef_LIB_SRCS})

target_link_libraries(ktnef  ${LIB_KIO} )

set_target_properties(ktnef PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} ktnef )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/ktnef $(all_includes)
#
#lib_LTLIBRARIES = libktnef.la
#
#libktnef_la_SOURCES = ktnefparser.cpp ktnefproperty.cpp ktnefattach.cpp mapi.cpp \
#	ktnefpropertyset.cpp ktnefmessage.cpp ktnefwriter.cpp lzfu.cpp
#libktnef_la_LIBADD = $(LIB_KDECORE) $(LIB_KIO)
#libktnef_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -no-undefined
#libktnef_la_METASOURCES = AUTO
#
#noinst_HEADERS = mapi.h lzfu.h
