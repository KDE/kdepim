INCLUDES= $(PISOCK_INCLUDE) $(PERL_CFLAGS) -I$(top_srcdir)/kpilot/lib $(all_includes)

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = python-conduit.desktop

kde_module_LTLIBRARIES = conduit_python.la


conduit_python_la_SOURCES = pythonconduit.kcfgc python-setup.ui \
	python-conduit.cc python-factory.cc
conduit_python_la_LDFLAGS = -module $(KDE_PLUGIN)  $(all_libraries)
conduit_python_la_LIBADD = ../../lib/libkpilot.la $(LIB_KDEUI)

