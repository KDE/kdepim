SUBDIRS=holidays

METASOURCES = AUTO

INCLUDES = -I$(top_srcdir) $(all_includes)

lib_LTLIBRARIES = libkholidays_ng.la

noinst_LTLIBRARIES = libholidayscommon.la
libholidayscommon_la_SOURCES = kde-holidays.cpp kde-holidays_parser.cpp

libkholidays_ng_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 2:0:0
libkholidays_ng_la_LIBADD = $(LIB_KIO)
libkholidays_ng_la_SOURCES = kholidays.cpp \
        holiday.cpp holidaycalendar.cpp holidayfilter.cpp \
        lunarphase.cpp astroseasons.cpp zodiac.cpp

check_PROGRAMS = dumpholidaylist benchmarkholidaylist

dumpholidaylist_LDFLAGS = $(all_libraries) $(KDE_RPATH)
dumpholidaylist_LDADD   = libholidayscommon.la $(LIB_KDECORE)
dumpholidaylist_SOURCES = dumpholidaylist.cpp
dumpholidaylist_COMPILE_FIRST = kde-holidays.h

benchmarkholidaylist_LDFLAGS = $(all_libraries) $(KDE_RPATH)
benchmarkholidaylist_LDADD   = libholidayscommon.la $(LIB_KDECORE)
benchmarkholidaylist_SOURCES = benchmarkholidaylist.cpp
benchmarkholidaylist_COMPILE_FIRST = kde-holidays.h

kde-holidays.cpp: kde-holidays.h
kde-holidays_parser.h kde-holidays_parser.cpp: kde-holidays.h

kde-holidays.h kde-holidays.cpp kde-holidays_parser.h kde-holidays_parser.cpp: $(srcdir)/kde-holidays.rng \
  $(top_builddir)/kode/kxml_compiler/kxml_compiler
	$(top_builddir)/kode/kxml_compiler/kxml_compiler \
	  --external-parser $(srcdir)/kde-holidays.rng

DISTCLEANFILES = kde-holidays.h kde-holidays.cpp \
	kde-holidays_parser.h kde-holidays_parser.cpp \
	kde-holidays.h.backup kde-holidays.cpp.backup \
	kde-holidays_parser.h.backup kde-holidays_parser.cpp.backup

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/libkholidays_ng.pot

include $(top_srcdir)/admin/Doxyfile.am
