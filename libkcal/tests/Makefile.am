INCLUDES = -I$(top_srcdir)/libkcal -I$(top_builddir)/libkcal/libical/src \
	-Iqttestlib $(all_includes)


check_PROGRAMS = loadcalendar \
                 testtostring \
                 testincidence \
                 testcalendar \
                 readandwrite \
                 testresource \
                 testfields \
                 testrecurrence \
                 testrecurprevious \
                 testrecurson \
                 testrecurrencetype \
		 testicaltimezones \
                 testvcalexport


METASOURCES = AUTO

loadcalendar_SOURCES = loadcalendar.cpp
loadcalendar_LDFLAGS = $(all_libraries) $(KDE_RPATH)
loadcalendar_LDADD = ../libkcal.la

testincidence_SOURCES = testincidence.cpp
testincidence_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testincidence_LDADD = ../libkcal.la

testcalendar_SOURCES = testcalendar.cpp
testcalendar_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testcalendar_LDADD = ../libkcal.la

testtostring_SOURCES = testtostring.cpp
testtostring_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testtostring_LDADD = ../libkcal.la

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

testicaltimezones_SOURCES = testicaltimezones.cpp
testicaltimezones_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testicaltimezones_LDADD = ../libkcal.la -lQtTest_debug

testvcalexport_SOURCES = testvcalexport.cpp
testvcalexport_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testvcalexport_LDADD = ../libkcal.la

testfields_SOURCES = testfields.cpp
testfields_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testfields_LDADD   = ../libkcal.la
testfields_CXXFLAGS= -DINPUT='"$(srcdir)/data/test_pilot.ics"'

TESTFILES = test1.ics test2.ics test3.ics test4.ics test5.ics test_Mozilla.ics

check-local: readandwrite testrecurrence testrecurprevious testrecurson testvcalexport testicaltimezones
	rm -f FAILED; \
	for i in `find $(srcdir)/data/RecurrenceRule/ -name "*.ics"`; do \
	  perl $(srcdir)/runtestcase.pl testrecurrence "next" $$i; \
	done; \
	for i in `find $(srcdir)/data/RecurrenceRule/ -name "*.ics"`; do \
	  perl $(srcdir)/runtestcase.pl testrecurprevious "prev" $$i; \
	done; \
	for i in `find $(srcdir)/data/RecurrenceRule/ -name "*.ics"`; do \
	  perl $(srcdir)/runtestcase.pl testrecurson "recurson" $$i; \
	done; \
	for i in `find $(srcdir)/data/Compat/ -name "*.ics"`; do \
	  perl $(srcdir)/runtestcase.pl readandwrite "ical" $$i; \
	done; \
	for i in `find $(srcdir)/data/vCalendar/ -name "*.ics"`; do \
		perl $(srcdir)/runtestcase.pl testvcalexport "vcal" $$i; \
	done;
	for i in `find $(srcdir)/data/vCalendar/ -name "*.vcs"`; do \
		perl $(srcdir)/runtestcase.pl readandwrite "ical" $$i; \
	done;
	[ ! -e FAILED ]

