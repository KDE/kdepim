

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")

# Is there really a version of STL still in use without the std:: namespace?
# For now we'll do without a configure check.
add_definitions(-DSTD_NAMESPACE_PREFIX="std::" -DHAVE_STLNAMESPACE)

include_directories( ${ZLIB_INCLUDE_DIR} )

macro_bool_to_01(BOOST_FOUND HAVE_BOOST)
configure_file (config-indexlib.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-indexlib.h )

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
install(TARGETS index  DESTINATION ${LIB_INSTALL_DIR})

set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix ${BIN_INSTALL_DIR})
set(libdir ${LIB_INSTALL_DIR})
set(includedir ${INCLUDE_INSTALL_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/indexlib-config.in ${CMAKE_CURRENT_BINARY_DIR}/indexlib-config @ONLY)

########### install files ###############

install( FILES create.h index.h lockfile.h  DESTINATION ${INCLUDE_INSTALL_DIR}/index)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/indexlib-config DESTINATION ${BIN_INSTALL_DIR} )

