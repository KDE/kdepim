INCLUDES = -I$(top_srcdir)/kab2/lib -I$(top_srcdir)/kab2/server $(all_includes)

bin_PROGRAMS = test addentries testparser

test_SOURCES        = test.cpp
addentries_SOURCES  = addentries.cpp
testparser_SOURCES  = testparser.cpp

METASOURCES         = AUTO

test_LDADD = \
  $(all_libraries) \
  $(top_builddir)/kab2/lib/libKAddressBook.la \
  $(top_builddir)/kab2/server/libKAddressBookServer.la \
  $(LIB_KDEUI)

addentries_LDADD = \
  $(all_libraries) \
  $(top_builddir)/kab2/lib/libKAddressBook.la \
  $(top_builddir)/kab2/server/libKAddressBookServer.la \
  $(LIB_KDECORE)

testparser_LDADD = \
  $(all_libraries) \
  $(top_builddir)/kab2/lib/libKAddressBook.la \
  $(top_builddir)/kab2/server/libKAddressBookServer.la \
  $(LIB_KDECORE)

