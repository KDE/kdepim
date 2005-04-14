noinst_LTLIBRARIES = libicalvcal.la

INCLUDES = -I../libical -I../libicalss

libicalvcal_la_SOURCES =	\
	icalvcal.h		\
	icalvcal.c		\
	port.h			\
	vcc.h			\
	vobject.h		\
	vobject.c		\
	vcaltmp.h		\
	vcaltmp.c		\
	vcc.c

libicalvcal_la_COMPILE_FIRST = ../libical/ical.h

#libicalvcalinclude_HEADERS =	\
#	icalvcal.h		\
#	port.h			\
#	vcc.h			\
#	vobject.h		\
#	vcaltmp.h

EXTRA_DIST =		\
	README.TXT	\
	vcaltest.c	\
	vctest.c
