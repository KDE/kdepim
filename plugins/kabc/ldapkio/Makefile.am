
# these are the headers for your project
noinst_HEADERS = resourceldapkioconfig.h

lib_LTLIBRARIES = libkabc_newldapkio.la

if need_kde32_compatibility
newldapkio_extra_libs = -L$(top_builddir)/libpimldaptools -lpimldaptools
newldapkio_extra_includes = -I$(top_srcdir)/libpimldaptools
endif

INCLUDES = -I$(top_srcdir)/kabc -I$(top_builddir)/kabc $(all_includes) $(newldapkio_extra_includes)

libkabc_newldapkio_la_SOURCES = resourceldapkio.cpp resourceldapkioconfig.cpp
libkabc_newldapkio_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0:0 -no-undefined
libkabc_newldapkio_la_LIBADD	= $(LIB_KABC) $(LIB_KIO) $(newldapkio_extra_libs)

kde_module_LTLIBRARIES = kabc_newldapkio.la
kabc_newldapkio_la_SOURCES = resourceldapkioplugin.cpp
kabc_newldapkio_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN)
kabc_newldapkio_la_LIBADD	= libkabc_newldapkio.la

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabc_newldapkio.pot

kabcincludedir = $(includedir)/kabc
# kabcinclude_HEADERS = resourceldapkio.h

servicedir = $(kde_servicesdir)/kresources/kabc
service_DATA = newldapkio.desktop

#resourceldapkioplugin.lo: $(kabcincludedir)/addressee.h
