lib_LTLIBRARIES = libdummykonnector.la

INCLUDES = -I$(top_srcdir)/ -I$(top_srcdir)/kitchensync/lib -I$(top_srcdir)/kitchensync/libkonnector2 -I$(top_srcdir)/libkcal $(all_includes)

libdummykonnector_la_INCLUDES = -I$(top_srcdir)/kitchensync/libkonnector2 -I$(top_srcdir)/kitchensync/lib -I../../   $(all_includes)
libdummykonnector_la_LIBADD = ../../libkonnector.la ../../../libksync2.la
libdummykonnector_la_LDFLAGS = $(all_libraries)  $(KDE_RPATH) -module
libdummykonnector_la_SOURCES = dummykonnector.cpp

servicedir = $(kde_servicesdir)/kresources/konnector
service_DATA = dummykonnector.desktop

METASOURCES = AUTO
