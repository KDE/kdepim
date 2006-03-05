kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libkleo ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kpimidentities_LIB_SRCS identitymanager.cpp identity.cpp identitycombo.cpp )

kde4_automoc(${kpimidentities_LIB_SRCS})

kde4_add_dcop_skels(kpimidentities_LIB_SRCS identitymanager.h )

kde4_add_library(kpimidentities SHARED ${kpimidentities_LIB_SRCS})

target_link_libraries(kpimidentities  ${QT_AND_KDECORE_LIBS} kleopatra kdepim emailfunctions )

set_target_properties(kpimidentities PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kpimidentities )


########### install files ###############


kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libkleo $(all_includes)
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
#lib_LTLIBRARIES = libkpimidentities.la
#libkpimidentities_la_SOURCES = \
#    identitymanager.cpp identity.cpp identitymanager.skel \
#    identitycombo.cpp
#libkpimidentities_la_LIBADD = ../libkleo/libkleopatra.la \
#    ../libkdepim/libkdepim.la ../libemailfunctions/libemailfunctions.la
#libkpimidentities_la_LDFLAGS = $(all_libraries) -no-undefined \
#    -version-info 1:0:0
#
#METASOURCES = AUTO
#
#include $(top_srcdir)/admin/Doxyfile.am
#
## No messages target. Those files are part of libkdepim.pot.
