INCLUDES = -I$(top_srcdir)/kab2/lib -I$(top_srcdir)/kab2/server $(all_includes)

bin_PROGRAMS = kab_client

kab_client_SOURCES = kab_client.cpp

kab_client_LDADD = \
  $(all_libraries) \
  $(top_builddir)/kab2/lib/libKAddressBook.la \
  $(LIB_KDECORE)

