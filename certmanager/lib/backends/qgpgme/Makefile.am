INCLUDES = -I$(top_srcdir)/libkdenetwork \
	-I$(top_srcdir)/certmanager/lib \
	$(GPGME_CFLAGS) $(all_includes)

noinst_LTLIBRARIES = libkleopatra_backend_qgpgme.la

libkleopatra_backend_qgpgme_la_SOURCES = \
			qgpgmekeylistjob.cpp \
			qgpgmekeygenerationjob.cpp \
			qgpgmeimportjob.cpp \
			qgpgmeexportjob.cpp \
			qgpgmedownloadjob.cpp \
			qgpgmedeletejob.cpp \
			qgpgmeencryptjob.cpp \
			qgpgmedecryptjob.cpp \
			qgpgmesignjob.cpp \
			qgpgmeverifydetachedjob.cpp \
			qgpgmeverifyopaquejob.cpp \
			qgpgmesignencryptjob.cpp \
			qgpgmedecryptverifyjob.cpp

#libkleopatra_backend_qgpgme_la_LIBADD = ../../kleo/libkleopatra_core.la
#libkleopatra_backend_qgpgme_la_LDFLAGS = $(all_libraries)

METASOURCES = AUTO
