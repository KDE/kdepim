INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = knotes-conduit.desktop

kde_module_LTLIBRARIES = conduit_knotes.la


conduit_knotes_la_SOURCES = knotesconduitSettings.kcfgc knotes-factory.cc \
	knotes-setup.cc knotes-action.cc \
	setup_base.ui \
	KNotesIface.stub
conduit_knotes_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
conduit_knotes_la_LIBADD = ../../lib/libkpilot.la $(LIB_KDEUI)

kde_kcfg_DATA = knotesconduit.kcfg
