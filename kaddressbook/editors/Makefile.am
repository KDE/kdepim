INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kaddressbook/interfaces \
           -I$(top_srcdir)/kaddressbook \
	   -I$(top_srcdir)/certmanager/lib \
	   -I$(top_srcdir)/libkdenetwork \
	   $(GPGME_CFLAGS) \
	   $(all_includes) 

kde_module_LTLIBRARIES = libkaddrbk_cryptosettings.la

XXLIBS = $(top_builddir)/kaddressbook/interfaces/libkabinterfaces.la \
         $(top_builddir)/certmanager/lib/libkleopatra.la

libkaddrbk_cryptosettings_la_SOURCES = cryptowidget.cpp

libkaddrbk_cryptosettings_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) # -no-undefined
libkaddrbk_cryptosettings_la_LIBADD = $(XXLIBS)

noinst_HEADERS = cryptowidget.h

METASOURCES = AUTO

servicedir  = $(kde_servicesdir)/kaddressbook
service_DATA = cryptosettings.desktop
