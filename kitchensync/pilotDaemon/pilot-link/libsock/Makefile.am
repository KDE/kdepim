SOCKOBJS = socket.c serial.c inetserial.c slp.c padp.c \
       dlp.c cmp.c syspkt.c inet.c money.c

FILEOBJS = pi-file.c

PIHEADER = pi-header.c

APPOBJS = datebook.c memo.c address.c todo.c expense.c mail.c sync.c appinfo.c hinote.c

UTILOBJS = utils.c util.c

MD5 = md5.c

SERIAL = unixserial.c

OBJS = $(SOCKOBJS) $(FILEOBJS) $(APPOBJS) $(PIHEADER) $(UTILOBJS) $(MD5) $(SERIAL)

lib_LTLIBRARIES = libpisock.la

libpisock_la_SOURCES = $(OBJS)
