#SUBDIRS = test

lib_LTLIBRARIES= libkonnector.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kitchensync/libksync $(all_includes)

libkonnector_la_LDFLAGS = $(all_libraries) $(KDE_EXTRA_RPATH) $(KDE_RPATH) \
                          -version-info 0:0 -no-undefined
libkonnector_la_LIBADD = $(top_builddir)/kitchensync/libksync/libksync2.la
libkonnector_la_SOURCES = filter.cpp idhelper.cpp konnector.cpp \
                          konnectorinfo.cpp konnectormanager.cpp

servicetype_DATA = konnector.desktop filter.desktop
servicetypedir = $(kde_servicetypesdir)

METASOURCES = AUTO
SUBDIRS = .  plugins test
ksincludedir      = $(includedir)/kitchensync/
ksinclude_HEADERS = filter.h idhelper.h \
                    konnector.h konnectorinfo.h konnectormanager.h
