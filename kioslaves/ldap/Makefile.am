## Makefile.am of kdepim/kioslaves/ldap

INCLUDES   = $(all_includes) $(LDAP_INCS)
AM_CXXFLAGS = -DLDAP_DEPRECATED
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LDAP_RPATH)
LDADD      = $(LIB_KIO) $(LDAP_LIBS)

####### Files

kde_module_LTLIBRARIES = kio_newldap.la

kio_newldap_la_SOURCES = kio_ldap.cpp
kio_newldap_la_LIBADD = $(LIB_KIO) $(LDAP_LIBS) $(LIB_KABC)
kio_newldap_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LDAP_RPATH) -module $(KDE_PLUGIN)

noinst_HEADERS = kio_ldap.h

kdelnk_DATA = newldap.protocol newldaps.protocol
kdelnkdir = $(kde_servicesdir)

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kio_newldap.pot
