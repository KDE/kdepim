SUBDIRS = versit tests

INCLUDES = -I$(top_srcdir)/libical/src/libical \
  -I$(top_srcdir)/libical/src/libicalss -I$(top_builddir)/libical/src/libical \
  -I$(top_builddir)/libical/src/libicalss -I$(srcdir)/versit $(all_includes)

lib_LTLIBRARIES   = libkcal.la
libkcal_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 2:0:0 -module
libkcal_la_LIBADD  = $(LIB_QT) versit/libversit.la $(LIB_KFILE) \
                     $(top_builddir)/libical/src/libical/libical.la \
                     $(top_builddir)/libical/src/libicalss/libicalss.la

libkcal_la_SOURCES = \
        incidencebase.cpp incidence.cpp journal.cpp todo.cpp event.cpp \
        freebusy.cpp attendee.cpp recurrence.cpp alarm.cpp \
        calendar.cpp calendarlocal.cpp \
        calformat.cpp vcalformat.cpp icalformat.cpp icalformatimpl.cpp \
	vcaldrag.cpp icaldrag.cpp \
        exceptions.cpp \
        scheduler.cpp imipscheduler.cpp dummyscheduler.cpp \
        calfilter.cpp \
        person.cpp period.cpp duration.cpp \
        dndfactory.cpp \
        filestorage.cpp \
        compat.cpp

METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp -o $(podir)/libkcal.pot

include ../admin/Doxyfile.am
