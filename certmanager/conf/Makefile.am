AM_CPPFLAGS = -I$(srcdir)/../lib -I$(srcdir)/../lib/ui -I../lib/ui -I$(top_srcdir)/libkdenetwork $(GPGME_CFLAGS) $(all_includes)

METASOURCES = AUTO

# the stuff that gets compiled into kgpgcertmanager
noinst_LTLIBRARIES = libconf.la
libconf_la_SOURCES = configuredialog.cpp

kde_module_LTLIBRARIES = kcm_kgpgcertmanager.la

kcm_kgpgcertmanager_la_SOURCES = configuredialog_p.cpp
kcm_kgpgcertmanager_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_kgpgcertmanager_la_LIBADD = ../lib/libkleopatra.la $(LIB_KDEUI)

kde_services_DATA = kgpgcertmanager_config_dirserv.desktop
