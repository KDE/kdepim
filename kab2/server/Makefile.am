INCLUDES                          = -I$(top_srcdir)/kab2/lib $(all_includes)

lib_LTLIBRARIES                   = libKAddressBookServer.la

libKAddressBookServer_la_SOURCES  = \
  KAddressBookServerInterface.cpp   \
  KAddressBookServerInterface.skel  \
  KAddressBookServerInterface.stub

libKAddressBookServer_la_LIBADD   = \
  $(top_builddir)/kab2/lib/libKAddressBook.la \
  $(LIB_KDECORE)

bin_PROGRAMS                      = kab_server

kab_server_LDADD                  = libKAddressBookServer.la
kab_server_SOURCES                = main.cpp

METASOURCES                       = AUTO

servicetypedir                    = $(kde_servicetypesdir)
servicetype_DATA                  = kab_server.desktop

