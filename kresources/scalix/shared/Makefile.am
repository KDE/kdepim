INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kresources/lib $(all_includes)

noinst_HEADERS = resourcescalixbase.h scalixbase.h subresource.h

noinst_LTLIBRARIES = libresourcescalixshared.la

libresourcescalixshared_la_SOURCES = \
	resourcescalixbase.cpp kmailconnection.cpp scalixbase.cpp \
  subresource.cpp \
	kmailconnection.skel kmailicalIface.stub
libresourcescalixshared_la_METASOURCES = AUTO
libresourcescalixshared_la_LIBADD = $(top_builddir)/libkcal/libkcal.la $(top_builddir)/libkdepim/libkdepim.la ../../lib/libkgroupwaredav.la
libresourcescalixshared_la_LDFLAGS = -no-undefined

kmailicalIface_DCOPIDLNG = true

kmailicalIface_DIR = $(top_srcdir)/kmail
