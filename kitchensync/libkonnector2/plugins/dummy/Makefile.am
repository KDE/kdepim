lib_LTLIBRARIES = libdummykonnector.la

INCLUDES = -I$(top_srcdir)/ -I$(top_srcdir)/kitchensync/libksync -I$(top_srcdir)/kitchensync/libkonnector2 -I$(top_srcdir)/libkcal $(all_includes)

libdummykonnector_la_INCLUDES = -I$(top_srcdir)/kitchensync/libkonnector2 -I$(top_srcdir)/kitchensync/libksync -I../../   $(all_includes)
libdummykonnector_la_LIBADD = ../../libkonnector.la
libdummykonnector_la_LDFLAGS = -module $(KDE_PLUGIN)
libdummykonnector_la_SOURCES = dummykonnector.cpp

servicedir = $(kde_servicesdir)/kresources/konnector
service_DATA = dummykonnector.desktop

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/konnector_dummy.pot
