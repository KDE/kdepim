INCLUDES= -I$(srcdir)/../.. -I$(srcdir)/.. -I$(top_builddir)/libkdepim $(all_includes)

kde_module_LTLIBRARIES = kio_sieve.la

kio_sieve_la_SOURCES = sieve.cpp
kio_sieve_la_LIBADD  = $(LIB_KIO) $(SASL2_LIBS)
kio_sieve_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

noinst_HEADERS = sieve.h

kdelnk_DATA = sieve.protocol
kdelnkdir = $(kde_servicesdir)

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kio_sieve.pot
