INCLUDES            = -I$(top_srcdir)/kab2/lib $(all_includes)

bin_PROGRAMS        = kab_server

kab_server_LDADD    = $(top_builddir)/kab2/lib/libKAddressBook.la $(LIB_KDECORE)

kab_server_SOURCES  = \
  KAddressBookServer.cpp \
  KAddressBookServerInterface.skel \
  main.cpp

METASOURCES         = AUTO

servicetypedir      = $(kde_servicetypesdir)
servicetype_DATA    = kab_server.desktop

