INCLUDES = -I$(top_srcdir)/libkdenetwork \
	-I$(top_srcdir)/certmanager/lib \
	$(GPGME_CFLAGS) $(all_includes)

check_PROGRAMS = test_keylister test_keygen test_keyselectiondialog

test_keylister_SOURCES = test_keylister.cpp
test_keygen_SOURCES = test_keygen.cpp
test_keyselectiondialog_SOURCES = test_keyselectiondialog.cpp

LDADD = ../libkleopatra.la

METASOURCES = AUTO

