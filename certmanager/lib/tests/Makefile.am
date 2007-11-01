AM_CPPFLAGS = -I$(top_srcdir)/certmanager/lib \
	-I$(top_srcdir)/libkdenetwork \
	$(GPGME_CFLAGS) $(all_includes) -DKDESRCDIR=\"$(srcdir)\"

check_PROGRAMS = test_keylister test_keygen test_keyselectiondialog \
		test_cryptoconfig test_gnupgprocessbase test_jobs test_verify
TESTS = test_verify

test_keylister_SOURCES = test_keylister.cpp
test_keygen_SOURCES = test_keygen.cpp
test_keyselectiondialog_SOURCES = test_keyselectiondialog.cpp
test_cryptoconfig_SOURCES = test_cryptoconfig.cpp
test_gnupgprocessbase_SOURCES = test_gnupgprocessbase.cpp
test_jobs_SOURCES = test_jobs.cpp
test_verify_SOURCES = test_verify.cpp

LDADD = ../libkleopatra.la

METASOURCES = AUTO

