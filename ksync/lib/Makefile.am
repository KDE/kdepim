INCLUDES = -I$(top_srcdir)/libkcal $(all_includes)

lib_LTLIBRARIES   = libksync.la
libksync_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0:0
libksync_la_LIBADD  = $(top_builddir)/libkcal/libkcal.la -lkabc $(LIB_QT)

libksync_la_SOURCES = \
  ksyncer.cpp ksyncui.cpp calendarsyncee.cpp bookmarksyncee.cpp ksyncuikde.cpp \
  addressbooksyncee.cpp

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/libksync.pot
