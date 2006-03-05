kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(index_LIB_SRCS 
   bitstream.cpp 
   compressed.cpp 
   create.cpp 
   exception.cpp 
   ifile.cpp 
   leafdata.cpp 
   leafdatavector.cpp 
   lockfile.cpp 
   logfile.cpp 
   match.cpp 
   mmap_manager.cpp 
   quotes.cpp 
   slow.cpp 
   stringarray.cpp 
   stringset.cpp 
   tokenizer.cpp )

kde4_automoc(${index_LIB_SRCS})

kde4_add_library(index SHARED ${index_LIB_SRCS})

target_link_libraries(index  ${QT_AND_KDECORE_LIBS} )

set_target_properties(index PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} index )


########### install files ###############

install_files( /include/index FILES create.h index.h lockfile.h )

kde4_footer()



#original Makefile.am contents follow:

#KDE_CXXFLAGS=-include $(srcdir)/compat.h $(USE_EXCEPTIONS)
#INCLUDES = $(all_includes)
#
#lib_LTLIBRARIES = libindex.la
#
#libindex_la_SOURCES = bitstream.cpp compressed.cpp create.cpp exception.cpp ifile.cpp leafdata.cpp leafdatavector.cpp lockfile.cpp logfile.cpp match.cpp mmap_manager.cpp quotes.cpp slow.cpp stringarray.cpp stringset.cpp tokenizer.cpp
#
#indexlibincludedir=$(includedir)/index
#indexlibinclude_HEADERS = create.h index.h lockfile.h
#bin_SCRIPTS = indexlib-config
#
