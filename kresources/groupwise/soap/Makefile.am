INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kresources/groupwise $(all_includes)

bin_PROGRAMS = soaptest

soaptest_LDFLAGS = $(all_libraries) $(KDE_RPATH)
soaptest_LDADD   = libgwsoap.la -lkdecore \
                   $(top_builddir)/libkcal/libkcal.la \
                   $(top_builddir)/libkdepim/libkdepim.la
soaptest_SOURCES = soaptest.cpp

noinst_LTLIBRARIES = libgwsoap.la

CPPFLAGS = -DDEBUG

libgwsoap_la_SOURCES = contactconverter.cpp gwconverter.cpp incidenceconverter.cpp \
                       groupwiseserver.cpp \
                       soapC.cpp soapClient.cpp stdsoap2.cpp
libgwsoap_la_LDFLAGS = $(KDE_RPATH) $(all_libraries)
libgwsoap_la_LIBADD	= -lkabc

stubs: groupwise.wsdl
	wsdl2h -o $(srcdir)/groupwise.h $(srcdir)/groupwise.wsdl -t $(srcdir)/typemap.dat
	soapcpp2 -d $(srcdir) -I$(srcdir) $(srcdir)/groupwise.h
	sed  -e 's,*delete\([^d]\),*_delete\1,' $(srcdir)/soapStub.h > $(srcdir)/soapStub.h_
	mv $(srcdir)/soapStub.h_ $(srcdir)/soapStub.h
	sed  -e 's,->delete\([^d]\),->_delete\1,' $(srcdir)/soapC.cpp > $(srcdir)/soapC.cpp_
	mv $(srcdir)/soapC.cpp_ $(srcdir)/soapC.cpp

METASOURCES = AUTO
