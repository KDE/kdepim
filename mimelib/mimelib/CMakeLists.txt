kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( /include/mimelib FILES  )

kde4_footer()



#original Makefile.am contents follow:

## $Id: Makefile.am 365583 2004-11-23 19:32:17Z dfaure $
#
#mimelibdir = $(includedir)/mimelib
#
#mimelib_HEADERS=    \
#   address.h \
#   addrlist.h \
#   body.h \
#   bodypart.h \
#   boyermor.h \
#   config.h \
#   datetime.h \
#   debug.h \
#   disptype.h \
#   entity.h \
#   enum.h \
#   field.h \
#   fieldbdy.h \
#   group.h \
#   headers.h \
#   mailbox.h \
#   mboxlist.h \
#   mechansm.h \
#   mediatyp.h \
#   message.h \
#   mimepp.h \
#   msgcmp.h \
#   msgid.h \
#   nntp.h \
#   param.h \
#   pop.h \
#   protocol.h \
#   string.h \
#   text.h \
#   token.h \
#   utility.h \
#   uuencode.h \
#   binhex.h
#
