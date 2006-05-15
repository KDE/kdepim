INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = memofile-conduit.desktop

kde_module_LTLIBRARIES = conduit_memofile.la


conduit_memofile_la_SOURCES = memofileSettings.kcfgc setup_base.ui \
	memofile-factory.cc memofile.cc memofiles.cc memofile-conduit.cc
conduit_memofile_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
conduit_memofile_la_LIBADD = ../../lib/libkpilot.la $(LIB_KDEUI) $(LIB_KFILE)

kde_kcfg_DATA = memofileconduit.kcfg
