INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kaddressbook/interfaces \
           -I$(top_srcdir)/kaddressbook \
           -I$(top_srcdir)/certmanager/lib \
           -I$(top_srcdir)/libkdenetwork \
	   -I$(top_srcdir)/libkdepim \
           -I$(top_builddir)/kaddressbook/editors \
           $(GPGME_CFLAGS) \
           $(all_includes)

kde_module_LTLIBRARIES = libkaddrbk_cryptosettings.la

XXLIBS = $(top_builddir)/kaddressbook/interfaces/libkabinterfaces.la \
         $(top_builddir)/certmanager/lib/libkleopatra.la

libkaddrbk_cryptosettings_la_SOURCES = cryptowidget.cpp
libkaddrbk_cryptosettings_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) # -no-undefined
libkaddrbk_cryptosettings_la_LIBADD = $(XXLIBS)

noinst_HEADERS = cryptowidget.h imeditorwidget.h imaddresswidget.h

METASOURCES = AUTO

SUBDIRS = protocols icons
servicedir  = $(kde_servicesdir)/kaddressbook
service_DATA = cryptosettings.desktop

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kaddressbookimprotocol.desktop

noinst_LTLIBRARIES = libkabim.la
libkabim_la_SOURCES = imeditorwidget.cpp imeditorbase.ui imaddressbase.ui imaddresswidget.cpp
libkabim_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined
libkabim_la_LIBADD = $(top_builddir)/kaddressbook/interfaces/libkabinterfaces.la
