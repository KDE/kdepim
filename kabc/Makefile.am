INCLUDES = $(all_includes)

lib_LTLIBRARIES   = libkabc.la
libkabc_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0:0 -module
libkabc_la_LIBADD  = $(LIB_KDECORE)

libkabc_la_SOURCES = \
  addressbook.cpp addressee.cpp phonenumber.cpp simpleformat.cpp \
  stdaddressbook.cpp

METASOURCES = AUTO

check_PROGRAMS = testkabc

testkabc_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testkabc_LDADD = libkabc.la $(LIB_KDECORE)
testkabc_SOURCES = testkabc.cpp
