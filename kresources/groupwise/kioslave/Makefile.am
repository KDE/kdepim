INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kresources/groupwise/soap \
  $(all_includes)

noinst_HEADERS = groupwise.h

METASOURCES = AUTO

kdelnkdir = $(kde_servicesdir)
kdelnk_DATA = groupwise.protocol groupwises.protocol

kde_module_LTLIBRARIES =  kio_groupwise.la

kio_groupwise_la_SOURCES = groupwise.cpp
kio_groupwise_la_LIBADD = $(top_builddir)/libkcal/libkcal.la \
  ../soap/libgwsoap.la $(top_builddir)/libkdepim/libkdepim.la $(LIB_KIO)
kio_groupwise_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

