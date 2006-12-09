INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = record-conduit.desktop

kde_module_LTLIBRARIES = conduit_record.la


conduit_record_la_SOURCES = settings.kcfgc setup_base.ui factory.cc
conduit_record_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
conduit_record_la_LIBADD = ../../lib/libkpilot.la $(LIB_KDEUI)

kde_kcfg_DATA = settings.kcfg
