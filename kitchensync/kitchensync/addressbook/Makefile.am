INCLUDES = -I$(top_srcdir)/kitchensync/kitchensync/lib -I$(top_srcdir)/kitchensync/libksync -I$(top_srcdir)/kitchensync/libkonnector2/  $(all_includes)

kde_module_LTLIBRARIES = libaddressbookpart.la
libaddressbookpart_la_LDFLAGS = $(all_libraries) $(LIB_KPARTS) $(KDE_PLUGIN) $(LIB_KABC)
libaddressbookpart_la_LIBADD =  ../lib/libkitchensyncui.la  

libaddressbookpart_la_SOURCES = addressbookbase.ui addressbookpart.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = addressbook.desktop

