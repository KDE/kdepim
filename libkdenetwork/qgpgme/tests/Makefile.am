INCLUDES = -I$(top_srcdir)/libkdenetwork $(GPGME_CFLAGS) $(all_includes)

check_PROGRAMS = dataprovidertest

TESTS = $(check_PROGRAMS)

dataprovidertest_SOURCES = dataprovidertest.cpp

LDADD = ../libqgpgme.la #?$(LIB_QT)
