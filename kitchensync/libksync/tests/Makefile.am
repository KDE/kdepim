INCLUDES = -I$(top_srcdir)/kitchensync/libksync -I$(top_srcdir) $(all_includes)

check_PROGRAMS = synctest1

METASOURCES = AUTO

synctest1_SOURCES = synctest1.cpp
synctest1_LDFLAGS = $(all_libraries) $(KDE_RPATH)
synctest1_LDADD = ../libksync2.la $(top_builddir)/libkcal/libkcal.la

check: synctest1
	mkdir -p ./data
	./synctest1 ./data
	perl $(srcdir)/checksync.pl ./data $(srcdir)/data 
