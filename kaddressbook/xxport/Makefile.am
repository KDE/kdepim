INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kaddressbook $(all_includes) 

kde_module_LTLIBRARIES = libkaddrbk_csv_xxport.la libkaddrbk_vcard_xxport.la \
libkaddrbk_kde2_xxport.la

noinst_HEADERS = importdialog.h

servicedir  = $(kde_servicesdir)/kaddressbook
service_DATA = csv_xxport.desktop vcard_xxport.desktop kde2_xxport.desktop

libkaddrbk_csv_xxport_la_SOURCES = csv_xxport.cpp importdialog.cpp
libkaddrbk_csv_xxport_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries)
libkaddrbk_csv_xxport_la_LIBADD = $(top_builddir)/kaddressbook/libkaddressbookpart.la
libkaddrbk_csv_xxport_la_METASOURCES = AUTO

libkaddrbk_vcard_xxport_la_SOURCES = vcard_xxport.cpp
libkaddrbk_vcard_xxport_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries)
libkaddrbk_vcard_xxport_la_LIBADD = $(top_builddir)/kaddressbook/libkaddressbookpart.la
libkaddrbk_vcard_xxport_la_METASOURCES = AUTO

libkaddrbk_kde2_xxport_la_SOURCES = kde2_xxport.cpp
libkaddrbk_kde2_xxport_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries)
libkaddrbk_kde2_xxport_la_LIBADD = $(top_builddir)/kaddressbook/libkaddressbookpart.la
libkaddrbk_kde2_xxport_la_METASOURCES = AUTO

rc_DATA = csv_xxportui.rc vcard_xxportui.rc kde2_xxportui.rc
rcdir = $(kde_datadir)/kaddressbook
