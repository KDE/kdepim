lib_LTLIBRARIES = libkcalkonnector.la

INCLUDES = -I$(top_srcdir)/ -I$(top_srcdir)/kitchensync/libksync \
           -I$(top_srcdir)/kitchensync/libkonnector2 $(all_includes)

libkcalkonnector_la_INCLUDES = -I$(top_srcdir)/kitchensync/libkonnector2 -I$(top_srcdir)/kitchensync/libksync -I../../   $(all_includes)
libkcalkonnector_la_LIBADD = $(top_builddir)/kitchensync/libkonnector2/libkonnector.la \
                             $(top_builddir)/libkcal/libkcal.la \
                             $(top_builddir)/libkdepim/libkdepim.la
libkcalkonnector_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkcalkonnector_la_SOURCES = kcalkonnector.cpp kcalkonnectorconfig.cpp

kcalkonnectordatadir = $(kde_servicesdir)/kresources/konnector
kcalkonnectordata_DATA = kcalkonnector.desktop

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/konnector_kcal.pot
