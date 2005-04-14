
noinst_LTLIBRARIES = libicalss.la

AM_YFLAGS =-d -v -p ss
AM_LFLAGS = -Pss
LEX_OUTPUT_ROOT = lex.ss

INCLUDES = -I../libical -I$(srcdir)/../libical

libicalss_la_SOURCES =		\
	icalcalendar.c		\
	icalcalendar.h		\
	icalclassify.c		\
	icalclassify.h		\
	icalcluster.c		\
	icalcluster.h		\
	icalclusterimpl.h	\
	icalgauge.c		\
	icalgauge.h		\
	icalgaugeimpl.h		\
	icaldirset.c		\
	icaldirset.h		\
	icaldirsetimpl.h	\
	icalfileset.c		\
	icalfileset.h		\
	icalfilesetimpl.h	\
	icalset.c		\
	icalset.h		\
	icalssyacc.h		\
	icalspanlist.c		\
	icalspanlist.h		\
	icalmessage.c		\
	icalmessage.h		\
	icalsslexer.c           \
        icalssyacc.c

# We don't need icalss.h, but it needs to be built...
libicalss_la_COMPILE_FIRST = ../libical/ical.h icalss.h

COMBINEDHEADERS = \
	$(srcdir)/icalgauge.h \
	$(srcdir)/icalset.h \
	$(srcdir)/icalcluster.h \
	$(srcdir)/icalfileset.h \
	$(srcdir)/icaldirset.h \
	$(srcdir)/icalcalendar.h  \
	$(srcdir)/icalclassify.h \
	$(srcdir)/icalspanlist.h	\
	$(srcdir)/icalmessage.h

icalss.h: $(COMBINEDHEADERS)
	echo '#ifdef __cplusplus'	  >  icalss.h
	echo 'extern "C" {'		  >> icalss.h
	echo '#endif'			  >> icalss.h
	echo '/*'			  >> icalss.h
	echo ' $$''Id''$$'		  >> icalss.h
	echo '*/'			  >> icalss.h
	cat $(COMBINEDHEADERS)				\
	| egrep -v "#include.*\"ical"			\
	| egrep -v "#include.*\"pvl\.h\""		\
	| egrep -v '\$$(Id|Locker): .+\$$'>> icalss.h
	echo '#ifdef __cplusplus'	  >> icalss.h
	echo '};'			  >> icalss.h
	echo '#endif'			  >> icalss.h

noinst_HEADERS = icalss.h icalcalendar.h icalclassify.h icalcluster.h \
                 icaldirset.h icaldirsetimpl.h icalfileset.h icalfilesetimpl.h \
                 icalgauge.h icalgaugeimpl.h icalmessage.h icalset.h \
                 icalspanlist.h icalssyacc.h

CLEANFILES = y.output icalss.h

