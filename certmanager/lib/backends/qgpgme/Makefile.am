#KDE_OPTIONS = nofinal

INCLUDES = -I$(top_srcdir)/libkdenetwork \
	-I$(top_srcdir)/certmanager/lib \
	-I$(top_builddir)/libkdepim \
	$(GPGME_CFLAGS) $(all_includes)

noinst_LTLIBRARIES = libkleopatra_backend_qgpgme.la

libkleopatra_backend_qgpgme_la_SOURCES = \
			gnupgprocessbase.cpp \
			qgpgmeprogresstokenmapper.cpp \
			\
			qgpgmebackend.cpp \
			\
			qgpgmejob.cpp \
			\
			qgpgmekeylistjob.cpp \
			qgpgmekeygenerationjob.cpp \
			qgpgmeimportjob.cpp \
			qgpgmeexportjob.cpp \
			qgpgmesecretkeyexportjob.cpp \
			qgpgmedownloadjob.cpp \
			qgpgmedeletejob.cpp \
			qgpgmeencryptjob.cpp \
			qgpgmedecryptjob.cpp \
			qgpgmesignjob.cpp \
			qgpgmeverifydetachedjob.cpp \
			qgpgmeverifyopaquejob.cpp \
			qgpgmesignencryptjob.cpp \
			qgpgmedecryptverifyjob.cpp \
			qgpgmecryptoconfig.cpp \
			qgpgmerefreshkeysjob.cpp

METASOURCES = AUTO
