SUBDIRS = . tests

INCLUDES = -I$(top_srcdir)/libkdenetwork $(all_includes)

lib_LTLIBRARIES = libqgpgme.la

qgpgmedir = $(includedir)/qgpgme
qgpgme_HEADERS = eventloopinteractor.h dataprovider.h

libqgpgme_la_SOURCES = eventloopinteractor.cpp dataprovider.cpp
libqgpgme_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined
libqgpgme_la_LIBADD = $(LIB_QT) ../gpgmepp/libgpgme++.la

METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/libqgpgme.pot

include $(top_srcdir)/doc/api/Doxyfile.am
