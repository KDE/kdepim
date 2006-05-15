

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")

# Is there really a version of STL still in use without the std:: namespace?
# For now we'll do without a configure check.
add_definitions(-DSTD_NAMESPACE_PREFIX="std::" -DHAVE_STLNAMESPACE)

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

target_link_libraries(index  ${KDE4_KDECORE_LIBS} )

set_target_properties(index PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} index )

set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix ${CMAKE_INSTALL_PREFIX}/bin)
set(libdir ${CMAKE_INSTALL_PREFIX}${LIB_INSTALL_DIR})
set(includedir ${CMAKE_INSTALL_PREFIX}/include)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/indexlib-config.in ${CMAKE_CURRENT_BINARY_DIR}/indexlib-config @ONLY)

########### install files ###############

install_files( /include/index FILES create.h index.h lockfile.h )
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/indexlib-config DESTINATION bin )



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
