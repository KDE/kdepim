SUBDIRS=versit holidays

INCLUDES = -I$(top_srcdir)/libical/src/libical \
  -I$(top_srcdir)/libical/src/libicalss -I$(top_builddir)/libical/src/libical \
  -I$(top_builddir)/libical/src/libicalss -I$(srcdir)/versit $(all_includes)

lib_LTLIBRARIES   = libkcal.la
libkcal_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0:0 -module
libkcal_la_LIBADD  = $(LIB_QT) versit/libversit.la $(LIB_KFILE) \
                     $(top_builddir)/libical/src/libical/libical.la \
                     $(top_builddir)/libical/src/libicalss/libicalss.la

libkcal_la_SOURCES = \
        incidence.cpp journal.cpp todo.cpp event.cpp attendee.cpp \
        korecurrence.cpp koalarm.cpp \
        calendar.cpp calendarlocal.cpp \
        calformat.cpp vcalformat.cpp icalformat.cpp icalformatimpl.cpp \
	vcaldrag.cpp \
        koexceptions.cpp \
        scheduler.cpp imipscheduler.cpp dummyscheduler.cpp \
        calfilter.cpp \
        scanholiday.c parseholiday.c \
        person.cpp

METASOURCES = AUTO

$(srcdir)/parseholiday.c: parseholiday.y
	cd $(srcdir) && bison -p kcal -d -o parseholiday.c parseholiday.y

$(srcdir)/parseholiday.h: parseholiday.y
	cd $(srcdir) && bison -p kcal -d -o parseholiday.c parseholiday.y

$(srcdir)/scanholiday.c: scanholiday.lex parseholiday.h
	cd $(srcdir) && flex -Pkcal -B -oscanholiday.c scanholiday.lex

