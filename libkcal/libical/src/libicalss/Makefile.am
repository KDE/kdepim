
noinst_LTLIBRARIES = libicalss.la

AM_YFLAGS =-d -v -p ss
AM_LFLAGS = -Pss
LEX_OUTPUT_ROOT = lex.ss

# just to get it built
$(srcdir)/icalgauge.c: icalssyacc.h
icalsslexer.lo: icalssyacc.h icalgaugeimpl.h

INCLUDES =			\
	-I$(top_builddir)/libkcal/libical	\
	-I$(top_srcdir)/libkcal/libical/src	\
	-I$(top_builddir)/libkcal/libical/src	\
	-I$(top_srcdir)/libkcal/libical/src/libical	\
	-I$(top_builddir)/libkcal/libical/src/libical	\
	-I$(top_srcdir)/libkcal/libical

libicalss_la_SOURCES =		\
	icalclassify.c		\
	icalgauge.c		\
	icaldirset.c		\
	icalfileset.c		\
	icalset.c		\
	icalssyacc.y		\
	icalsslexer.l		\
	icalspanlist.c		\
	icalmessage.c		\
	icalcstpclient.c        \
	icalcstpserver.c        \
	icalcstp.c		
libicalss_la_COMPILE_FIRST = ../libical/ical.h

COMBINEDHEADERS = \
        $(srcdir)/icalgauge.h \
        $(srcdir)/icalset.h \
        $(srcdir)/icalfileset.h \
        $(srcdir)/icaldirset.h \
        $(srcdir)/icalcalendar.h  \
        $(srcdir)/icalclassify.h \
	$(srcdir)/icalspanlist.h	\
	$(srcdir)/icalmessage.h \
	$(srcdir)/icalcstp.h \
	$(srcdir)/icalcstpclient.h \
	$(srcdir)/icalcstpserver.h

icalss.h: $(COMBINEDHEADERS) icalssyacc.c
	cat $(COMBINEDHEADERS) | egrep -v "#include.*\"ical" \
	| egrep -v "#include.*\"pvl\.h\"" > icalss.h

noinst_HEADERS = icalss.h

CLEANFILES = y.output icalss.h icalssyacc.c icalssyacc.h

EXTRA_DIST = $(COMBINEDHEADERS)

