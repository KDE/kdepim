SUBDIRS = . frontend

INCLUDES = -I$(top_srcdir)/vCard/include -I$(top_srcdir)/vCard/include/generated \
  $(all_includes)

lib_LTLIBRARIES   = libkabc.la
libkabc_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0:0 -module
libkabc_la_LIBADD  = $(top_builddir)/vCard/libvcard.la $(LIB_KDEUI)

libkabc_la_SOURCES = \
  addressbook.cpp addressee.cpp phonenumber.cpp simpleformat.cpp \
  stdaddressbook.cpp vcardformat.cpp address.cpp geo.cpp timezone.cpp \
  vcardformatimpl.cpp addresseedialog.cpp

METASOURCES = AUTO

bin_PROGRAMS = kab2kabc

kab2kabc_LD_FLAGS = $(all_libraries) $(KDE_RPATH)
kab2kabc_LDADD = libkabc.la $(LIB_KAB)
kab2kabc_SOURCES = kab2kabc.cpp

check_PROGRAMS = testkabc testkabcdlg

testkabc_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testkabc_LDADD = libkabc.la
testkabc_SOURCES = testkabc.cpp

testkabcdlg_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testkabcdlg_LDADD = libkabc.la
testkabcdlg_SOURCES = testkabcdlg.cpp
