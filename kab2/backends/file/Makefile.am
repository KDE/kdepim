INCLUDES = -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libkabbackend_file.la

libkabbackend_file_la_LIBADD = \
  $(top_builddir)/kab2/libKAddressBook.la \
  $(LIB_KDECORE)

libkabbackend_file_la_SOURCES = KAddressBookFileBackend.cpp
libkabbackend_file_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)

libkabbackend_file_la_METASOURCES = AUTO

kde_services_DATA = KAddressBookFileBackend.desktop
