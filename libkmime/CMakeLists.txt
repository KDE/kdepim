
add_subdirectory( tests )

include_directories(
	${CMAKE_SOURCE_DIR}/libkdepim/
)

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
   kmime_contentindex.cpp
   kmime_headers.cpp 
   kmime_message.cpp 
   kmime_newsarticle.cpp 
   boolflags.cpp 
   kautodeletehash.cpp )

kde4_automoc(${kmime_LIB_SRCS})

kde4_add_library(kmime SHARED ${kmime_LIB_SRCS})

target_link_libraries(kmime  ${KDE4_KIO_LIBS} )

set_target_properties(kmime PROPERTIES VERSION 2.2.0 SOVERSION 2 )
install(TARGETS kmime  DESTINATION ${LIB_INSTALL_DIR})
