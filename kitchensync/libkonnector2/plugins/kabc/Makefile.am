lib_LTLIBRARIES = libkabckonnector.la

INCLUDES = -I$(top_srcdir)/ -I$(top_srcdir)/kitchensync/libksync -I$(top_builddir)/libkdepim \
           -I$(top_srcdir)/kitchensync/libkonnector2 $(all_includes)

libkabckonnector_la_INCLUDES = -I$(top_srcdir)/kitchensync/libkonnector2 -I$(top_srcdir)/kitchensync/libksync -I../../   $(all_includes)
libkabckonnector_la_LIBADD = ../../libkonnector.la -lkabc_file -lkabc
libkabckonnector_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkabckonnector_la_SOURCES = kabckonnector.cpp kabckonnectorconfig.cpp

kabckonnectordatadir = $(kde_servicesdir)/kresources/konnector
kabckonnectordata_DATA = kabckonnector.desktop

METASOURCES = AUTO
