INCLUDES = -I$(top_srcdir)/libkcal $(all_includes)

lib_LTLIBRARIES   = libksync.la
libksync_la_LDFLAGS = -lkonq $(all_libraries) -no-undefined -version-info 1:0:0 -module
libksync_la_LIBADD  = $(top_builddir)/libkcal/libkcal.la $(LIB_QT)

libksync_la_SOURCES = \
  ksyncer.cpp ksyncui.cpp calendarsyncee.cpp bookmarksyncee.cpp ksyncuikde.cpp

METASOURCES = AUTO
