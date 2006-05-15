### Makefile for the sysinfo conduit
###
### The sysinfo conduit is Copyright (C) 2003 by Reinhold Kainhofer

INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = sysinfo_conduit.desktop

kde_module_LTLIBRARIES = conduit_sysinfo.la

conduit_sysinfo_la_SOURCES = sysinfoSettings.kcfgc \
	sysinfo-factory.cc \
	sysinfo-setup.cc \
	sysinfo-conduit.cc \
	sysinfo-setup_dialog.ui
conduit_sysinfo_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
conduit_sysinfo_la_LIBADD = ../../lib/libkpilot.la $(LIB_KDEUI) $(LIB_KIO)

kpilot_sysinfo_data_DATA = Template.html Template.txt
kpilot_sysinfo_datadir = $(kde_datadir)/kpilot/sysinfoconduit
EXTRA_DIST = $(kpilot_sysinfo_data_DATA)
kde_kcfg_DATA = sysinfoconduit.kcfg
