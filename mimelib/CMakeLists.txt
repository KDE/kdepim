project(mimelib)

add_subdirectory( mimelib )
add_subdirectory( tests )

########### next target ###############

set(mimelib_LIB_SRCS
   protocol.cpp
   address.cpp
   addrlist.cpp
   body.cpp
   bodypart.cpp
   boyermor.cpp
   datetime.cpp
   disptype.cpp
   dw_cte.cpp
   dw_date.cpp
   dw_mime.cpp
   entity.cpp
   field.cpp
   fieldbdy.cpp
   group.cpp
   headers.cpp
   mailbox.cpp
   mboxlist.cpp
   mechansm.cpp
   mediatyp.cpp
   message.cpp
   msgcmp.cpp
   msgid.cpp
   nntp.cpp
   param.cpp
   pop.cpp
   dwstring.cpp
   text.cpp
   token.cpp
   uuencode.cpp
   binhex.cpp )


kde4_add_library(mimelib SHARED ${mimelib_LIB_SRCS})

target_link_libraries(mimelib  ${KDE4_KDECORE_LIBS})

set_target_properties(mimelib PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS mimelib ${INSTALL_TARGETS_DEFAULT_ARGS})
