INCLUDES = -I$(top_srcdir) $(all_includes)

bin_PROGRAMS = kab2test kab2testaddentries kab2testparser

kab2test_SOURCES             = kab2test.cpp
kab2testaddentries_SOURCES   = kab2testaddentries.cpp
kab2testparser_SOURCES       = kab2testparser.cpp

LDFLAGS = $(all_libraries)
METASOURCES = AUTO

kab2test_LDADD = \
  $(top_builddir)/kab2/libKAddressBook.la \
  $(top_builddir)/kab2/libKAddressBookServer.la \
  $(LIB_KDEUI)

kab2testaddentries_LDADD = \
  $(top_builddir)/kab2/libKAddressBook.la \
  $(top_builddir)/kab2/libKAddressBookServer.la \
  $(LIB_KDECORE)

kab2testparser_LDADD = \
  $(top_builddir)/kab2/libKAddressBook.la \
  $(top_builddir)/kab2/libKAddressBookServer.la \
  $(LIB_KDECORE)

