#SUBDIRS = networkstatustray

METASOURCES = AUTO

kde_module_LTLIBRARIES = kded_networkstatus.la
noinst_LTLIBRARIES = libnetworkstatus.la

libnetworkstatus_la_LIBADD = $(LIB_KDECORE)
libnetworkstatus_la_LDFLAGS = $(all_libraries)
libnetworkstatus_la_SOURCES = networkstatuscommon.cpp

INCLUDES = $(all_includes)

kded_networkstatus_la_SOURCES = networkstatus.cpp networkstatus.skel \
	clientiface.skel serviceiface.skel network.cpp 
#kclientiface.skel serviceiface.skel
kded_networkstatus_la_LIBADD = $(LIB_KDECORE) $(LIB_KIO) ./libnetworkstatus.la
kded_networkstatus_la_LDFLAGS = $(all_libraries) -module -avoid-version 

servicesdir = $(kde_servicesdir)/kded
services_DATA = networkstatus.desktop

noinst_HEADERS = serviceiface.h networkstatuscommon.h serviceifaceimpl.h \
	network.h clientifaceimpl.h provideriface.h testservice.h

bin_PROGRAMS = networkstatustestservice

networkstatustestservice_LDFLAGS = $(all_libraries)
networkstatustestservice_LDADD = libnetworkstatus.la

networkstatustestservice_SOURCES = testservice.cpp provideriface.skel serviceiface.stub
