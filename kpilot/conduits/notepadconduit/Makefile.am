INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = notepad-conduit.desktop

kde_module_LTLIBRARIES = conduit_notepad.la

conduit_notepad_la_SOURCES = notepadconduit.kcfgc notepad-setup.ui \
	notepad-conduit.cc notepad-factory.cc
conduit_notepad_la_LDFLAGS= -module $(KDE_PLUGIN) $(all_libraries)
conduit_notepad_la_LIBADD= ../../lib/libkpilot.la $(LIB_KDEUI) $(LIB_KIO)

