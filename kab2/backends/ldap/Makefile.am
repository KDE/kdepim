INCLUDES = -I$(top_srcdir)/kab2/lib $(all_includes) -I/usr/local/include

kde_module_LTLIBRARIES = libkabbackend_ldap.la

libkabbackend_ldap_la_LIBADD  = \
  $(top_builddir)/kab2/lib/libKAddressBook.la \
  $(LIB_KDECORE) \
  -llber \
  -lldap \
  -lresolv

libkabbackend_ldap_la_SOURCES = KAddressBookLDAPBackend.cpp
libkabbackend_ldap_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)

METASOURCES                   = AUTO

kde_services_DATA             = KAddressBookLDAPBackend.desktop
