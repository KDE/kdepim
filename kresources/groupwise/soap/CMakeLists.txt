set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/kresources/groupwise  )


########### next target ###############
set(soapdebug_SRCS soapdebug.cpp )


# todo: more appropriate icon?
#kde4_add_app_icon(soapdebug_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/kbugbuster.png")

kde4_add_executable(soapdebug TEST ${soapdebug_SRCS})

target_link_libraries(soapdebug  ${KDE4_KDECORE_LIBS} gwsoap ${KDE4_KCAL_LIBS} kdepim )

########### next target ###############

set(gwsoap_SRCS
   contactconverter.cpp
   gwconverter.cpp
   incidenceconverter.cpp
   groupwiseserver.cpp
   gwjobs.cpp
   soapC.cpp soapClient.cpp stdsoap2.cpp
   )

kde4_add_library(gwsoap SHARED ${gwsoap_SRCS})
target_link_libraries(gwsoap ${KDE4_KCAL_LIBS} ${KDE4_KABC_LIBS} ${KDE4_KIO_LIBS} )
set_target_properties(gwsoap PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS gwsoap DESTINATION ${LIB_INSTALL_DIR})

########### install files ###############





#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kresources/groupwise $(all_includes)
#
##AM_CPPFLAGS = -DDEBUG
#KDE_CXXFLAGS = -Wno-non-virtual-dtor
#
#check_PROGRAMS = soapdebug
#
#soapdebug_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#soapdebug_LDADD   = libgwsoap.la -lkdecore \
#                   $(top_builddir)/libkcal/libkcal.la \
#                   $(top_builddir)/libkdepim/libkdepim.la
#soapdebug_SOURCES = soapdebug.cpp
#
#noinst_LTLIBRARIES = libgwsoap.la
#
#libgwsoap_la_SOURCES = contactconverter.cpp gwconverter.cpp incidenceconverter.cpp \
#                       groupwiseserver.cpp gwjobs.cpp ksslsocket.cpp \
#                       soapC.cpp soapClient.cpp stdsoap2.cpp
#libgwsoap_la_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#libgwsoap_la_LIBADD	= -lkabc
#
## the following rule does:
## make a header file from the wsdl
## remove ns1__ prefixes from generated functions
## create the stubs from the header
## rename any 'delete' method declarations
## rename any 'delete' method invocations
## rename any 'new' method decls
## rename any 'new' method invocations
#
## do not just set "srcdir=." here, it breaks when building with srcdir!=builddir
## srcdir = .
#
#stubs: groupwise.wsdl
#	wsdl2h -e -o $(srcdir)/groupwise.h $(srcdir)/groupwise.wsdl -t $(srcdir)/typemap.dat
#	cd $(srcdir); ./fixup_groupwise.h.pl --inplace
#	soapcpp2 -d $(srcdir) -I$(srcdir) $(srcdir)/groupwise.h
#	sed  -e 's,*delete\([^d]\),*_delete\1,' $(srcdir)/soapStub.h > $(srcdir)/soapStub.h_
#	mv $(srcdir)/soapStub.h_ $(srcdir)/soapStub.h
#	sed  -e 's,->delete\([^d]\),->_delete\1,' $(srcdir)/soapC.cpp > $(srcdir)/soapC.cpp_
#	mv $(srcdir)/soapC.cpp_ $(srcdir)/soapC.cpp
#	sed  -e 's,*new\([^d]\),*_new\1,' $(srcdir)/soapStub.h > $(srcdir)/soapStub.h_
#	mv $(srcdir)/soapStub.h_ $(srcdir)/soapStub.h
#	sed  -e 's,->new\([^d]\),->_new\1,' $(srcdir)/soapC.cpp > $(srcdir)/soapC.cpp_
#	mv $(srcdir)/soapC.cpp_ $(srcdir)/soapC.cpp
#
#METASOURCES = AUTO
