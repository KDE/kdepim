INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kresources/groupwise $(all_includes)

#AM_CPPFLAGS = -DDEBUG
KDE_CXXFLAGS = -Wno-non-virtual-dtor 

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
	# make a header file from the wsdl
	wsdl2h -e -o $(srcdir)/groupwise.h $(srcdir)/groupwise-new.wsdl -t $(srcdir)/typemap.dat
	# remove ns1__ prefixes from generated functions
	cd $(srcdir); ./fixup_groupwise.h.pl --inplace
	# create the stubs from the header
	soapcpp2 -d $(srcdir) -I$(srcdir) $(srcdir)/groupwise.h
	# rename any 'delete' method declarations
	sed  -e 's,*delete\([^d]\),*_delete\1,' $(srcdir)/soapStub.h > $(srcdir)/soapStub.h_
	mv $(srcdir)/soapStub.h_ $(srcdir)/soapStub.h
	# rename any 'delete' method invocations
	sed  -e 's,->delete\([^d]\),->_delete\1,' $(srcdir)/soapC.cpp > $(srcdir)/soapC.cpp_
	mv $(srcdir)/soapC.cpp_ $(srcdir)/soapC.cpp
	# rename any 'new' method decls
	sed  -e 's,*new\([^d]\),*_new\1,' $(srcdir)/soapStub.h > $(srcdir)/soapStub.h_
	mv $(srcdir)/soapStub.h_ $(srcdir)/soapStub.h
	# rename any 'new' method invocations
	sed  -e 's,->new\([^d]\),->_new\1,' $(srcdir)/soapC.cpp > $(srcdir)/soapC.cpp_
	mv $(srcdir)/soapC.cpp_ $(srcdir)/soapC.cpp

METASOURCES = AUTO
