INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = null-conduit.desktop

kde_module_LTLIBRARIES = conduit_null.la


conduit_null_la_SOURCES = nullSettings.kcfgc setup_base.ui null-conduit.cc null-factory.cc
conduit_null_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
conduit_null_la_LIBADD = ../../lib/libkpilot.la $(LIB_KDEUI)

kde_kcfg_DATA = nullconduit.kcfg
