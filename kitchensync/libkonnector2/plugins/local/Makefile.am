lib_LTLIBRARIES = liblocalkonnector.la

INCLUDES = -I$(top_srcdir)/ -I$(top_srcdir)/kitchensync/lib -I$(top_srcdir)/kitchensync/libkonnector2 -I$(top_srcdir)/libkcal $(all_includes)

liblocalkonnector_la_INCLUDES = -I$(top_srcdir)/kitchensync/libkonnector2 -I$(top_srcdir)/kitchensync/lib -I../../   $(all_includes)
liblocalkonnector_la_LIBDADD = ../../libkonnector.la ../../../libksync2.la
liblocalkonnector_la_LDFLAGS = $(all_libraries)  $(KDE_RPATH) -module
liblocalkonnector_la_SOURCES = localkonnector.cpp localkonnectorconfig.cpp

localkonnectordatadir = $(kde_datadir)/kitchensync
localkonnectordata_DATA = localkonnector.desktop

METASOURCES = AUTO
