INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kresources/opengroupware/soap \
  $(all_includes)

noinst_HEADERS = opengroupware.h

METASOURCES = AUTO

kdelnkdir = $(kde_servicesdir)
kdelnk_DATA = opengroupware.protocol opengroupwares.protocol

kde_module_LTLIBRARIES =  kio_opengroupware.la

kio_opengroupware_la_SOURCES = opengroupware.cpp webdavhandler.cpp
kio_opengroupware_la_LIBADD = $(top_builddir)/libkcal/libkcal.la \
   $(top_builddir)/libkdepim/libkdepim.la $(LIB_KIO)
kio_opengroupware_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

