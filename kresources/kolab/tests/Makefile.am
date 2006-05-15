INCLUDES = -I$(top_srcdir)/libkolabformat $(all_includes)

check_PROGRAMS = testnotes

METASOURCES = AUTO

testnotes_SOURCES = testnotes.cpp
testnotes_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testnotes_LDADD = ../libkolabformat.la


check: readandwrite
	rm -f FAILED; \
        ./testnotes; \
	[ ! -e FAILED ]
