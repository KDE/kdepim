lib_LTLIBRARIES = liblocalkonnector.la

INCLUDES = -I$(top_srcdir)/ -I$(top_srcdir)/kitchensync/libksync -I$(top_srcdir)/kitchensync/libkonnector2 \
           -I$(top_srcdir)/libkcal $(all_includes)

liblocalkonnector_la_INCLUDES = -I$(top_srcdir)/kitchensync/libkonnector2 -I$(top_srcdir)/kitchensync/libksync -I../../   $(all_includes)
liblocalkonnector_la_LIBADD = ../../libkonnector.la -lkabc_file
liblocalkonnector_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
liblocalkonnector_la_SOURCES = localkonnector.cpp localkonnectorconfig.cpp

localkonnectordatadir = $(kde_servicesdir)/kresources/konnector
localkonnectordata_DATA = localkonnector.desktop

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/konnector_local.pot
