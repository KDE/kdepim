INCLUDES = -I$(top_srcdir) $(all_includes)

noinst_HEADERS = groupwise.h

METASOURCES = AUTO

kdelnkdir = $(kde_servicesdir)
kdelnk_DATA = groupwise.protocol

kde_module_LTLIBRARIES =  kio_groupwise.la

kio_groupwise_la_SOURCES = groupwise.cpp
kio_groupwise_la_LIBADD = $(top_builddir)/libkcal/libkcal.la $(LIB_KIO)
kio_groupwise_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

