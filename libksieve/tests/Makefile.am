
INCLUDES = -I$(top_srcdir)/libksieve $(all_includes)
LDADD = ../libksieve.la

# test programs:
check_PROGRAMS = \
	lexertest \
	parsertest

TESTS = $(check_PROGRAMS)

lexertest_SOURCES = lexertest.cpp
parsertest_SOURCES = parsertest.cpp
