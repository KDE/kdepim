INCLUDES = -I$(top_srcdir)/libical/src/libical \
           -I$(top_srcdir)/libical/src/libicalss \
           -I$(top_builddir)/libical/src/libical \
           -I$(top_builddir)/libical/src/libicalss \
           -I$(srcdir)/versit -I$(top_srcdir)/ \
           -I$(top_srcdir)/knotes \
           -I$(top_builddir)/kaddressbook \
           -I$(top_builddir)/kaddressbook/common \
           -I$(top_srcdir)/kaddressbook/common \
           -I$(top_builddir)/kresources/egroupware \
           $(all_includes)

noinst_HEADERS = kabc_resourcexmlrpcconfig.h kcal_resourcexmlrpcconfig.h \
                 knotes_resourcexmlrpcconfig.h

lib_LTLIBRARIES = libkabc_xmlrpc.la libkcal_xmlrpc.la libknotes_xmlrpc.la
libkabc_xmlrpc_la_SOURCES = kabc_resourcexmlrpc.cpp kabc_resourcexmlrpcconfig.cpp xmlrpciface.cpp uidmapper.cpp \
                            kabc_egroupwareprefs.kcfgc synchronizer.cpp
libkabc_xmlrpc_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0:0 -no-undefined
libkabc_xmlrpc_la_LIBADD = $(LIB_KIO) -lkabc $(top_builddir)/libkdepim/libkdepim.la \
                           $(top_builddir)/kaddressbook/common/libkabcommon.la

libkcal_xmlrpc_la_SOURCES = kcal_resourcexmlrpc.cpp kcal_resourcexmlrpcconfig.cpp xmlrpciface.cpp uidmapper.cpp \
                            kcal_egroupwareprefs.kcfgc synchronizer.cpp
libkcal_xmlrpc_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0:0 -no-undefined
libkcal_xmlrpc_la_LIBADD = $(top_builddir)/libkcal/libkcal.la $(top_builddir)/libkdepim/libkdepim.la

libknotes_xmlrpc_la_SOURCES = knotes_resourcexmlrpc.cpp knotes_resourcexmlrpcconfig.cpp xmlrpciface.cpp \
                              knotes_egroupwareprefs.kcfgc synchronizer.cpp
libknotes_xmlrpc_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0:0 -no-undefined
libknotes_xmlrpc_la_LIBADD = $(top_builddir)/libkcal/libkcal.la -lkdeprint \
                             $(top_builddir)/knotes/libknotesresources.la \
									  $(top_builddir)/libkdepim/libkdepim.la

kabcincludedir = $(includedir)/kabc
kabcinclude_HEADERS = kabc_resourcexmlrpc.h kcal_resourcexmlrpc.h

kde_module_LTLIBRARIES = kabc_xmlrpc.la kcal_xmlrpc.la knotes_xmlrpc.la

kabc_xmlrpc_la_SOURCES = kabc_resourcexmlrpcplugin.cpp
kabc_xmlrpc_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kabc_xmlrpc_la_LIBADD	= libkabc_xmlrpc.la

kcal_xmlrpc_la_SOURCES = kcal_resourcexmlrpcplugin.cpp
kcal_xmlrpc_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kcal_xmlrpc_la_LIBADD = libkcal_xmlrpc.la

knotes_xmlrpc_la_SOURCES = knotes_resourcexmlrpcplugin.cpp
knotes_xmlrpc_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
knotes_xmlrpc_la_LIBADD = libknotes_xmlrpc.la

kabc_servicedir = $(kde_servicesdir)/kresources/kabc
kabc_service_DATA = kabc_xmlrpc.desktop

kcal_servicedir = $(kde_servicesdir)/kresources/kcal
kcal_service_DATA = kcal_xmlrpc.desktop

knotes_servicedir = $(kde_servicesdir)/kresources/knotes
knotes_service_DATA = knotes_xmlrpc.desktop

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kres_xmlrpc.pot
