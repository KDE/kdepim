
noinst_LTLIBRARIES = libicalss.la

AM_YFLAGS =-d -v -p ss
AM_LFLAGS = -Pss
LEX_OUTPUT_ROOT = lex.ss

INCLUDES = -I../libical -I$(srcdir)/../libical

libicalss_la_SOURCES =		\
	icalclassify.c		\
	icalclassify.h

# We don't need icalss.h, but it needs to be built...
libicalss_la_COMPILE_FIRST = ../libical/ical.h icalss.h

COMBINEDHEADERS = \
	$(srcdir)/icalclassify.h 

icalss.h: $(COMBINEDHEADERS)
	echo '#ifdef __cplusplus'	  >  icalss.h
	echo 'extern "C" {'		  >> icalss.h
	echo '#endif'			  >> icalss.h
	cat $(COMBINEDHEADERS)				\
	| egrep -v "#include.*\"ical"			\
	| egrep -v "#include.*\"pvl\.h\""		\
	| egrep -v '\$$(Id|Locker): .+\$$'>> icalss.h
	echo '#ifdef __cplusplus'	  >> icalss.h
	echo '}'			  >> icalss.h
	echo '#endif'			  >> icalss.h

noinst_HEADERS = icalss.h icalclassify.h 

CLEANFILES = icalss.h

