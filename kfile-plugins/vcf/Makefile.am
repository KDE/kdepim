## Makefile.am for vcf file meta info plugin

# set the include path for X, qt and KDE
INCLUDES         = $(all_includes)

# these are the headers for your project
noinst_HEADERS   = kfile_vcf.h

kde_module_LTLIBRARIES = kfile_vcf.la

kfile_vcf_la_SOURCES = kfile_vcf.cpp 
kfile_vcf_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kfile_vcf_la_LIBADD = $(LIB_KSYCOCA)

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) kfile_vcf.cpp -o $(podir)/kfile_vcf.pot

services_DATA = kfile_vcf.desktop
servicesdir = $(kde_servicesdir)
