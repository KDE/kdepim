INCLUDES= -I$(top_builddir)/libkdepim $(all_includes)

####### Files

METASOURCES = AUTO

kde_module_LTLIBRARIES = kio_mbox.la

kio_mbox_la_SOURCES = \
	mbox.cc \
	mboxfile.cc \
	readmbox.cc \
	stat.cc \
	urlinfo.cc
kio_mbox_la_LIBADD  = $(LIB_KIO)
kio_mbox_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module $(KDE_PLUGIN)

noinst_HEADERS = \
	mbox.h \
	mboxfile.h \
	readmbox.h \
	stat.h \
	urlinfo.h

kdelnk_DATA = mbox.protocol
kdelnkdir = $(kde_servicesdir)


