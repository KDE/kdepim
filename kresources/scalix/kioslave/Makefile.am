INCLUDES = -I$(top_srcdir) -I$(top_builddir)/libkdepim $(all_includes)

noinst_HEADERS = scalix.h

METASOURCES = AUTO

kdelnkdir = $(kde_servicesdir)
kdelnk_DATA = scalix.protocol scalixs.protocol

kde_module_LTLIBRARIES =  kio_scalix.la

kio_scalix_la_SOURCES = scalix.cpp
kio_scalix_la_LIBADD = $(top_builddir)/libkcal/libkcal.la $(top_builddir)/libkdepim/libkdepim.la $(LIB_KIO)
kio_scalix_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kio_scalix.pot
