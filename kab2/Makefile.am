############################################################################

SUBDIRS = . backends tools # test

INCLUDES = -I$(top_srcdir) $(all_includes)

lib_LTLIBRARIES = libKAddressBook.la libKAddressBookServer.la
bin_PROGRAMS = kab_server

############################################################################

libKAddressBook_la_SOURCES = \
  Entry.cpp \
  Field.cpp \
  FieldFormat.cpp \
  FormatDefinition.cpp \
  Job.cpp \
  KAddressBookInterface.cpp \
  KAddressBookInterface.skel \
  KAddressBookInterface.stub \
  KAddressBookBackend.cpp

libKAddressBook_la_METASOURCES = AUTO

libKAddressBook_la_LIBADD = $(LIB_KDECORE)

kab2includedir = $(includedir)/kab2

kab2include_HEADERS =  \
  Command.h \
  Entry.h \
  Field.h \
  FieldFormat.h \
  FormatDefinition.h \
  KAddressBookBackend.h \
  KAddressBookInterface.h

libKAddressBook_la_INCLUDES = KAddressBookInterface.h KAddressBookBackend.h

############################################################################

libKAddressBookServer_la_SOURCES  = \
  KAddressBookServerInterface.cpp   \
  KAddressBookServerInterface.skel  \
  KAddressBookServerInterface.stub

libKAddressBookServer_la_METASOURCES = AUTO

libKAddressBookServer_la_LIBADD = ./libKAddressBook.la $(LIB_KDECORE)

kab2includedir = $(includedir)/kab2
kab2include_HEADERS = KAddressBookServerInterface.h

############################################################################

kab_server_LDADD = libKAddressBookServer.la
kab_server_SOURCES = kab_server.cpp

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kab_server.desktop

kab_server_METASOURCES = AUTO

###########################################################################

noinst_HEADERS = Job.h

############################################################################

formatdir = $(kde_datadir)/kab2/formats

install-data-local:
	$(mkinstalldirs) $(formatdir)
	$(INSTALL_DATA) $(srcdir)/default.kabformat $(formatdir)/default.kabformat

uninstall-local:
	-rm -f $(formatdir)/default.kabformat
  
