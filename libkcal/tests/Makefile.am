INCLUDES = -I$(top_srcdir)/libkcal $(all_includes)

check_PROGRAMS = testtostring testincidence testcalendar readandwrite

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

TESTFILES = test1.ics test2.ics test3.ics

check: readandwrite
	for i in $(TESTFILES); do \
	  perl $(srcdir)/checkcalendar.pl  $(srcdir)/data/$$i; \
        done
