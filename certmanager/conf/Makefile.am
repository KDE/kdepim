AM_CPPFLAGS = -DLIBKLEOPATRA_NO_COMPAT -I$(srcdir)/../lib -I$(srcdir)/../lib/ui -I../lib/ui \
   -I$(top_srcdir)/libkdenetwork $(GPGME_CFLAGS) $(all_includes)

METASOURCES = AUTO

# the stuff that gets compiled into kleopatra
noinst_LTLIBRARIES = libconf.la
libconf_la_SOURCES = configuredialog.cpp

kde_module_LTLIBRARIES = kcm_kleopatra.la

kcm_kleopatra_la_SOURCES = dirservconfigpage.cpp \
	appearanceconfigpage.cpp \
	appearanceconfigwidgetbase.ui \
	appearanceconfigwidget.cpp \
	dnorderconfigpage.cpp

kcm_kleopatra_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_kleopatra_la_LIBADD = ../lib/libkleopatra.la $(LIB_KDEUI)

kde_services_DATA = \
	kleopatra_config_dirserv.desktop \
	kleopatra_config_appear.desktop \
	kleopatra_config_dnorder.desktop
