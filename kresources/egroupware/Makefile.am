INCLUDES = -I$(top_srcdir)/libical/src/libical \
           -I$(top_srcdir)/libical/src/libicalss \
           -I$(top_builddir)/libical/src/libical \
           -I$(top_builddir)/libical/src/libicalss \
           -I$(srcdir)/versit -I$(top_srcdir)/ \
           $(all_includes)

noinst_HEADERS = kcal_resourcexmlrpcconfig.h kabc_resourcexmlrpcconfig.h

lib_LTLIBRARIES = libkabc_xmlrpc.la
libkabc_xmlrpc_la_SOURCES = kabc_resourcexmlrpc.cpp kabc_resourcexmlrpcconfig.cpp xmlrpciface.cpp
libkabc_xmlrpc_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0:0 -no-undefined
libkabc_xmlrpc_la_LIBADD = $(LIB_KIO) -lkabc

kabcincludedir = $(includedir)/kabc
kabcinclude_HEADERS = kabc_resourcexmlrpc.h

kde_module_LTLIBRARIES = kcal_xmlrpc.la kabc_xmlrpc.la

kcal_xmlrpc_la_SOURCES = kcal_resourcexmlrpc.cpp kcal_resourcexmlrpcconfig.cpp xmlrpciface.cpp
kcal_xmlrpc_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kcal_xmlrpc_la_LIBADD = $(top_builddir)/libkcal/libkcal.la

kabc_xmlrpc_la_SOURCES = kabc_resourcexmlrpcplugin.cpp
kabc_xmlrpc_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kabc_xmlrpc_la_LIBADD	= libkabc_xmlrpc.la

kcal_servicedir = $(kde_servicesdir)/kresources/kcal
kcal_service_DATA = kcal_xmlrpc.desktop

kabc_servicedir = $(kde_servicesdir)/kresources/kabc
kabc_service_DATA = kabc_xmlrpc.desktop

METASOURCES = AUTO
