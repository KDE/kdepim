INCLUDES = -I$(top_srcdir) $(all_includes)

noinst_HEADERS = resourcekolabbase.h kolabbase.h subresource.h

noinst_LTLIBRARIES = libresourcekolabshared.la

libresourcekolabshared_la_SOURCES = \
	resourcekolabbase.cpp kmailconnection.cpp kolabbase.cpp \
        subresource.cpp \
	kmailconnection.skel kmailicalIface.stub
libresourcekolabshared_la_METASOURCES = AUTO

kmailicalIface_DIR = $(top_srcdir)/kmail
