INCLUDES = -I$(top_srcdir)/libkdenetwork -I$(top_srcdir)/libkdepim \
	-I$(top_srcdir)/certmanager/lib \
	$(GPGME_CFLAGS) $(all_includes)

noinst_LTLIBRARIES = libkleopatra_core.la

libkleopatra_core_la_SOURCES = \
	cryptobackendfactory.cpp \
	enum.cpp \
	dn.cpp \
	job.cpp \
	\
	multideletejob.cpp \
	hierarchicalkeylistjob.cpp \
	\
	kconfigbasedkeyfilter.cpp \
	keyfiltermanager.cpp

kleodir = $(includedir)/kleo
kleo_HEADERS = \
	enum.h \
	cryptobackend.h \
	cryptobackendfactory.h \
	cryptoconfig.h \
	dn.h \
	job.h \
	keylistjob.h keygenerationjob.h importjob.h exportjob.h downloadjob.h deletejob.h \
	encryptjob.h decryptjob.h signjob.h verifydetachedjob.h verifyopaquejob.h \
	decryptverifyjob.h signencryptjob.h refreshkeysjob.h \
	multideletejob.h \
	hierarchicalkeylistjob.h \
	\
	keyfilter.h \
	keyfiltermanager.h

METASOURCES = AUTO
