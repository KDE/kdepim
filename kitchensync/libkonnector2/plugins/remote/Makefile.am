lib_LTLIBRARIES = libremotekonnector.la

INCLUDES = -I$(top_srcdir)/ -I$(top_srcdir)/kitchensync/libksync -I$(top_srcdir)/kitchensync/libkonnector2 \
           -I$(top_srcdir)/libkcal -I$(top_builddir)/libkdepim $(all_includes)

libremotekonnector_la_INCLUDES = -I$(top_srcdir)/kitchensync/libkonnector2 -I$(top_srcdir)/kitchensync/libksync -I../../   $(all_includes)
libremotekonnector_la_LIBADD = ../../libkonnector.la -lkabc_file
libremotekonnector_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libremotekonnector_la_SOURCES = remotekonnector.cpp remotekonnectorconfig.cpp

remotekonnectordatadir = $(kde_servicesdir)/kresources/konnector
remotekonnectordata_DATA = remotekonnector.desktop

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/konnector_remote.pot
