INCLUDES= $(PISOCK_INCLUDE) $(PERL_CFLAGS) -I$(top_srcdir)/kpilot/lib $(all_includes)

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = perl-conduit.desktop

kde_module_LTLIBRARIES = conduit_perl.la


conduit_perl_la_SOURCES = perlconduit.kcfgc perl-setup.ui \
	perl-conduit.cc perl-factory.cc
conduit_perl_la_LDFLAGS = -module $(KDE_PLUGIN) $(PERL_LDFLAGS) $(all_libraries)
conduit_perl_la_LIBADD = ../../lib/libkpilot.la $(LIB_KDEUI)

