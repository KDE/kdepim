INCLUDES = -I$(top_srcdir)/libkdenetwork \
	-I$(top_srcdir)/certmanager/lib \
	$(GPGME_CFLAGS) $(all_includes)

noinst_LTLIBRARIES = libkleopatra_core.la

libkleopatra_core_la_SOURCES = \
	dn.cpp \
	job.cpp

#libkleopatra_core_la_LIBADD = ../../../libkdenetwork/qgpgme/libqgpgme.la

kleodir = $(includedir)/kleo
kleo_HEADERS = \
	cryptobackend.h \
	\
	dn.h \
	job.h \
	keylistjob.h keygenerationjob.h importjob.h exportjob.h downloadjob.h deletejob.h \
	encryptjob.h decryptjob.h signjob.h verifydetachedjob.h verifyopaquejob.h

METASOURCES = AUTO
