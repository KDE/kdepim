INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kresources/groupwise $(all_includes)

#AM_CPPFLAGS = -DDEBUG
KDE_CXXFLAGS = -Wno-unused-parameter -Wno-non-virtual-dtor 

check_PROGRAMS = soapdebug

soapdebug_LDFLAGS = $(all_libraries) $(KDE_RPATH)
soapdebug_LDADD   = libgwsoap.la -lkdecore \
                   $(top_builddir)/libkcal/libkcal.la \
                   $(top_builddir)/libkdepim/libkdepim.la
soapdebug_SOURCES = soapdebug.cpp

noinst_LTLIBRARIES = libgwsoap.la

libgwsoap_la_SOURCES = contactconverter.cpp gwconverter.cpp incidenceconverter.cpp \
                       groupwiseserver.cpp gwjobs.cpp ksslsocket.cpp \
                       soapC.cpp soapClient.cpp stdsoap2.cpp
libgwsoap_la_LDFLAGS = $(KDE_RPATH) $(all_libraries)
libgwsoap_la_LIBADD	= -lkabc

stubs: groupwise.wsdl
	wsdl2h -e -o $(srcdir)/groupwise.h $(srcdir)/groupwise.wsdl -t $(srcdir)/typemap.dat
	cd $(srcdir); ./fixup_groupwise.h.pl --inplace
	soapcpp2 -d $(srcdir) -I$(srcdir) $(srcdir)/groupwise.h
	sed  -e 's,*delete\([^d]\),*_delete\1,' $(srcdir)/soapStub.h > $(srcdir)/soapStub.h_
	mv $(srcdir)/soapStub.h_ $(srcdir)/soapStub.h
	sed  -e 's,->delete\([^d]\),->_delete\1,' $(srcdir)/soapC.cpp > $(srcdir)/soapC.cpp_
	mv $(srcdir)/soapC.cpp_ $(srcdir)/soapC.cpp

METASOURCES = AUTO
