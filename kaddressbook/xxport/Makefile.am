INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kaddressbook $(all_includes) 

kde_module_LTLIBRARIES = libkaddrbk_csv_xxport.la libkaddrbk_vcard_xxport.la \
libkaddrbk_kde2_xxport.la libkaddrbk_bookmark_xxport.la \
libkaddrbk_eudora_xxport.la libkaddrbk_ldif_xxport.la libkaddrbk_gnokii_xxport.la

noinst_HEADERS = importdialog.h

servicedir  = $(kde_servicesdir)/kaddressbook
service_DATA = csv_xxport.desktop vcard_xxport.desktop kde2_xxport.desktop \
bookmark_xxport.desktop eudora_xxport.desktop ldif_xxport.desktop gnokii_xxport.desktop

METASOURCES = AUTO
AM_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries)

libkaddrbk_csv_xxport_la_SOURCES = csv_xxport.cpp importdialog.cpp
libkaddrbk_csv_xxport_la_LIBADD = $(top_builddir)/kaddressbook/libkaddressbook.la

libkaddrbk_vcard_xxport_la_SOURCES = vcard_xxport.cpp
libkaddrbk_vcard_xxport_la_LIBADD = $(top_builddir)/kaddressbook/libkaddressbook.la

libkaddrbk_kde2_xxport_la_SOURCES = kde2_xxport.cpp
libkaddrbk_kde2_xxport_la_LIBADD = $(top_builddir)/kaddressbook/libkaddressbook.la

libkaddrbk_bookmark_xxport_la_SOURCES = bookmark_xxport.cpp
libkaddrbk_bookmark_xxport_la_LIBADD = $(top_builddir)/kaddressbook/libkaddressbook.la

libkaddrbk_eudora_xxport_la_SOURCES = eudora_xxport.cpp
libkaddrbk_eudora_xxport_la_LIBADD = $(top_builddir)/kaddressbook/libkaddressbook.la

libkaddrbk_ldif_xxport_la_SOURCES = ldif_xxport.cpp
libkaddrbk_ldif_xxport_la_LIBADD = $(top_builddir)/kaddressbook/libkaddressbook.la

libkaddrbk_gnokii_xxport_la_SOURCES = gnokii_xxport.cpp
libkaddrbk_gnokii_xxport_la_LIBADD = $(XPMLIB) $(LIB_GNOKII) $(top_builddir)/kaddressbook/libkaddressbook.la

rc_DATA = csv_xxportui.rc vcard_xxportui.rc kde2_xxportui.rc \
bookmark_xxportui.rc eudora_xxportui.rc ldif_xxportui.rc gnokii_xxportui.rc
rcdir = $(kde_datadir)/kaddressbook
