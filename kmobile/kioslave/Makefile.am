SUBDIRS  = mimetypes

INCLUDES = -I$(top_srcdir)/kmobile -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = kio_mobile.la

kio_mobile_la_SOURCES = kio_mobile.cpp 
kio_mobile_la_LIBADD = $(LIB_KIO) ../libkmobileclient.la
kio_mobile_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module $(KDE_PLUGIN)

kde_services_DATA = mobile.protocol

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kio_mobile.pot
