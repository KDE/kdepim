INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libemailfunctions $(all_includes)


lib_LTLIBRARIES = libkslox.la libkabc_slox.la libkcal_slox.la

libkslox_la_SOURCES = sloxaccounts.cpp webdavhandler.cpp
libkslox_la_LDFLAGS = $(all_libraries)
libkslox_la_LIBADD  = $(top_builddir)/libkcal/libkcal.la \
                      $(top_builddir)/libkdepim/libkdepim.la

libkcal_slox_la_SOURCES = kcalresourceslox.cpp kcalresourcesloxconfig.cpp \
                          kcalsloxprefs.kcfgc confirmsavedialog.cpp
libkcal_slox_la_LDFLAGS = $(all_libraries)
libkcal_slox_la_LIBADD  = libkslox.la $(top_builddir)/libkcal/libkcal.la

libkabc_slox_la_SOURCES = kabcresourceslox.cpp kabcresourcesloxconfig.cpp \
                          kabcsloxprefs.kcfgc
libkabc_slox_la_LDFLAGS = $(all_libraries)
libkabc_slox_la_LIBADD  = libkslox.la  -lkabc


kde_module_LTLIBRARIES = kcal_slox.la kabc_slox.la

kcal_slox_la_SOURCES = kcalresourceslox_plugin.cpp
kcal_slox_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kcal_slox_la_LIBADD  = libkcal_slox.la

kabc_slox_la_SOURCES = kabcresourceslox_plugin.cpp
kabc_slox_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kabc_slox_la_LIBADD  = libkabc_slox.la


EXTRA_PROGRAMS = testsloxaccounts

testsloxaccounts_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testsloxaccounts_LDADD = libkslox.la
testsloxaccounts_SOURCES = testsloxaccounts.cpp


kcal_servicedir = $(kde_servicesdir)/kresources/kcal
kcal_service_DATA = kcal_slox.desktop

kabc_servicedir = $(kde_servicesdir)/kresources/kabc
kabc_service_DATA = kabc_slox.desktop

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabc_slox.pot
