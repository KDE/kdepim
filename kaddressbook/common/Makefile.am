INCLUDES= -I$(top_srcdir)/kaddressbook/interfaces \
          -I$(top_srcdir) -I$(top_builddir)/libkdepim \
          $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libkabcommon.la
libkabcommon_la_SOURCES = kabprefs.cpp kabprefs_base.kcfgc locationmap.cpp filter.cpp 
libkabcommon_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined
libkabcommon_la_LIBADD = $(top_builddir)/kaddressbook/interfaces/libkabinterfaces.la \
                         $(top_builddir)/libkdepim/libkdepim.la
noinst_HEADERS = filter.h kabprefs.h 
