INCLUDES = -I$(top_srcdir)/libkcal $(all_includes)

check_PROGRAMS = testtostring testincidence testcalendar readandwrite testresource

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

readandwrite_SOURCES = readandwrite.cpp
readandwrite_LDFLAGS = $(all_libraries) $(KDE_RPATH)
readandwrite_LDADD = ../libkcal.la

testresource_SOURCES = testresource.cpp
testresource_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testresource_LDADD = ../libkcal.la

TESTFILES = test1.ics test2.ics test3.ics test4.ics test5.ics test_Mozilla.ics

check: readandwrite
	rm -f FAILED; \
	for i in $(TESTFILES); do \
	  perl $(srcdir)/checkcalendar.pl  $(srcdir)/data/$$i; \
        done; \
	[ ! -e FAILED ]
