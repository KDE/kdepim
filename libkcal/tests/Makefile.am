INCLUDES = -I$(top_srcdir)/libkcal \
  -I$(srcdir)/../libical/src/libical \
  -I$(srcdir)/../libical/src/libicalss -I../libical/src/libical \
  -I../libical/src/libicalss -I$(srcdir)/../versit \
  -I$(top_srcdir) $(all_includes)

AM_CPPFLAGS = -DKDETOPSRCDIR=\"$(top_srcdir)\"

check_PROGRAMS = testtostring \
                 testincidence \
                 testcalendar \
                 fbrecurring \
                 readandwrite \
                 testresource \
                 testfields \
                 testrecurrence \
                 testrecurprevious \
                 testrecurson \
                 testrecurrencetype \
                 testvcalexport \
                 testfb


METASOURCES = AUTO

testincidence_SOURCES = testincidence.cpp
testincidence_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testincidence_LDADD = ../libkcal.la

testcalendar_SOURCES = testcalendar.cpp
testcalendar_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testcalendar_LDADD = ../libkcal.la

testtostring_SOURCES = testtostring.cpp
testtostring_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testtostring_LDADD = ../libkcal.la

fbrecurring_SOURCES = fbrecurring.cpp
fbrecurring_LDFLAGS = $(all_libraries) $(KDE_RPATH)
fbrecurring_LDADD = ../libkcal.la

readandwrite_SOURCES = readandwrite.cpp
readandwrite_LDFLAGS = $(all_libraries) $(KDE_RPATH)
readandwrite_LDADD = ../libkcal.la

testresource_SOURCES = testresource.cpp
testresource_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testresource_LDADD = ../libkcal.la

testrecurrence_SOURCES = testrecurrence.cpp
testrecurrence_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testrecurrence_LDADD = ../libkcal.la

testrecurprevious_SOURCES = testrecurprevious.cpp
testrecurprevious_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testrecurprevious_LDADD = ../libkcal.la

testrecurson_SOURCES = testrecurson.cpp
testrecurson_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testrecurson_LDADD = ../libkcal.la

testrecurrencetype_SOURCES = testrecurrencetype.cpp
testrecurrencetype_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testrecurrencetype_LDADD = ../libkcal.la

testvcalexport_SOURCES = testvcalexport.cpp
testvcalexport_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testvcalexport_LDADD = ../libkcal.la

testfields_SOURCES = testfields.cpp
testfields_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testfields_LDADD   = ../libkcal.la
testfields_CXXFLAGS= -DINPUT='"$(srcdir)/data/test_pilot.ics"'

testfb_SOURCES = testfb.cpp
testfb_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testfb_LDADD = ../libkcal.la

TESTFILES = test1.ics test2.ics test3.ics test4.ics test5.ics test_Mozilla.ics

check-local: readandwrite testrecurrence testrecurprevious testrecurson testvcalexport
	rm -f FAILED;
	for i in `find $(srcdir)/data/RecurrenceRule/ -name "*.ics"`; do \
	  perl $(srcdir)/runtestcase.pl testrecurrence "next" $$i; \
	done;
	for i in `find $(srcdir)/data/RecurrenceRule/ -name "*.ics"`; do \
	  perl $(srcdir)/runtestcase.pl testrecurprevious "prev" $$i; \
	done;
	for i in `find $(srcdir)/data/RecurrenceRule/ -name "*.ics"`; do \
	  perl $(srcdir)/runtestcase.pl testrecurson "recurson" $$i; \
	done;
#	for i in `find $(srcdir)/data/Compat/ -name "*.ics"`; do \
#	  perl $(srcdir)/runtestcase.pl readandwrite "ical" $$i; \
#	done;
	for i in `find $(srcdir)/data/vCalendar/ -name "*.ics"`; do \
	  perl $(srcdir)/runtestcase.pl testvcalexport "vcal" $$i; \
	done;
	for i in `find $(srcdir)/data/vCalendar/ -name "*.vcs"`; do \
	  perl $(srcdir)/runtestcase.pl readandwrite "ical" $$i; \
	done;
	[ ! -e FAILED ]

