# UNIX makefile for MIME++ library and example programs

# Choose a version to compile.  I recommend compiling development version,
# which includes numerous assert macros to catch bad function arguments
# and other safeguards.  The production version is designed to avoid
# program aborts, such as will occur if an assertion fails.  The production
# version tries to recover as best it can in the case of exceptional
# conditions.  The debug version is designed to help you find bugs once
# you know they exist.  The development version helps you out here just
# a little, because it will dump core so you can examine the program
# state with a debugger.
#
# Make sure you type 'make clean' after compiling one version before
# compiling a different version.

SUBDIRS = mimelib

LIBVERSION = DW_DEVELOPMENT_VERSION
# LIBVERSION = DW_PRODUCTION_VERSION
# LIBVERSION = DW_DEBUG_VERSION

AM_CPPFLAGS = -D$(LIBVERSION) -D_REENTRANT
lib_LTLIBRARIES = libmimelib.la

libmimelib_la_SOURCES = \
    protocol.cpp \
    address.cpp \
    addrlist.cpp \
    body.cpp \
    bodypart.cpp \
    boyermor.cpp \
    datetime.cpp \
    disptype.cpp \
    dw_cte.cpp \
    dw_date.cpp \
    dw_mime.cpp \
    entity.cpp \
    field.cpp \
    fieldbdy.cpp \
    group.cpp \
    headers.cpp \
    mailbox.cpp \
    mboxlist.cpp \
    mechansm.cpp \
    mediatyp.cpp \
    message.cpp \
    msgcmp.cpp \
    msgid.cpp \
    nntp.cpp \
    param.cpp \
    pop.cpp \
    dwstring.cpp \
    text.cpp \
    token.cpp \
    uuencode.cpp \
    binhex.cpp

libmimelib_la_LDFLAGS = -version-info 1:1 -no-undefined

check_PROGRAMS = test_boyermor

TESTS=$(check_PROGRAMS)

test_boyermor_SOURCES = test_boyermor.cpp
test_boyermor_LDADD = libmimelib.la $(LIBSOCKET)
