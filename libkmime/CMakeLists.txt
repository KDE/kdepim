kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libemailfunctions ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kmime_LIB_SRCS 
   kmime_charfreq.cpp 
   kmime_util.cpp 
   kmime_mdn.cpp 
   kmime_codecs.cpp 
   kmime_codec_base64.cpp 
   kmime_codec_uuencode.cpp 
   kmime_codec_qp.cpp 
   kmime_codec_identity.cpp 
   kmime_parsers.cpp 
   kmime_header_parsing.cpp 
   kmime_content.cpp 
   kmime_headers.cpp 
   kmime_message.cpp 
   kmime_newsarticle.cpp 
   boolflags.cpp 
   kautodeletehash.cpp )

kde4_automoc(${kmime_LIB_SRCS})

kde4_add_library(kmime SHARED ${kmime_LIB_SRCS})

target_link_libraries(kmime  ${QT_AND_KDECORE_LIBS} emailfunctions )

set_target_properties(kmime PROPERTIES VERSION 2.2.0 SOVERSION 2 )
install_targets(${LIB_INSTALL_DIR} kmime )


########### install files ###############


kde4_create_doxygen_docs( EXCLUDE kmime_headers_obs.h  )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= -I$(top_srcdir)/libemailfunctions \
#        $(all_includes)
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
#lib_LTLIBRARIES = libkmime.la
#
#libkmime_la_SOURCES = \
#			  kmime_charfreq.cpp \
#			  kmime_util.cpp \
#			  kmime_mdn.cpp \
#			  kmime_codecs.cpp \
#			  kmime_codec_base64.cpp \
#			  kmime_codec_uuencode.cpp \
#			  kmime_codec_qp.cpp \
#			  kmime_codec_identity.cpp \
#			  kmime_parsers.cpp \
#			  kmime_header_parsing.cpp \
#			  kmime_content.cpp \
#			  kmime_headers.cpp \
#			  kmime_message.cpp \
#			  kmime_newsarticle.cpp \
#			  boolflags.cpp \
#			  kautodeletehash.cpp
#
#libkmime_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 4:0:2
#
#libkmime_la_LIBADD = $(top_builddir)/libemailfunctions/libemailfunctions.la \
#                     $(LIB_KDECORE) $(LIB_POLL)
#
#METASOURCES = AUTO
#
#messages: 
#	$(XGETTEXT) *.cpp *.h -o $(podir)/libkmime.pot
#
#DOXYGEN_EXCLUDE =  kmime_headers_obs.h
#include $(top_srcdir)/admin/Doxyfile.am
