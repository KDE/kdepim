INCLUDES = -I$(top_srcdir)/libkdenetwork \
	-I$(top_srcdir)/certmanager/lib \
	-I$(top_srcdir)/libkpgp \
	$(GPGME_CFLAGS) $(all_includes)

noinst_LTLIBRARIES = libkleopatra_backend_kpgp.la

libkleopatra_backend_kpgp_la_SOURCES = \
			kpgpbackendbase.cpp kpgpwrapper.cpp \
                        kpgpkeylistjob.cpp

METASOURCES = AUTO
