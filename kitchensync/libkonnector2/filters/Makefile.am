kde_module_LTLIBRARIES = libksfilter_addressbook.la libksfilter_calendar.la

INCLUDES = -I$(top_srcdir)/ -I$(top_srcdir)/kitchensync/libksync \
           -I$(top_srcdir)/kitchensync/libkonnector2 $(all_includes)

libksfilter_addressbook_la_LIBADD = $(top_builddir)/kitchensync/libkonnector2/libkonnector.la -lkabc
libksfilter_addressbook_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libksfilter_addressbook_la_SOURCES = addressbookfilter.cpp

libksfilter_calendar_la_LIBADD = $(top_builddir)/kitchensync/libkonnector2/libkonnector.la $(top_builddir)/libkcal/libkcal.la
libksfilter_calendar_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libksfilter_calendar_la_SOURCES = calendarfilter.cpp

ksfilterdatadir = $(kde_servicesdir)/kitchensync/filter
ksfilterdata_DATA = addressbookfilter.desktop calendarfilter.desktop

METASOURCES = AUTO
