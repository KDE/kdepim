noinst_LTLIBRARIES = libprinter.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kaddressbook \
           -I$(top_srcdir)/kaddressbook/details $(all_includes) 

libprinter_la_METASOURCES = AUTO

libprinter_la_SOURCES = printingwizard_base.ui \
	printingwizard.cpp \
	printstyle.cpp \
	printprogress_base.ui printprogress.cpp \
	detailledstyle.cpp  \
	ds_appearance.ui \
	mikesstyle.cpp 

EXTRA_DIST = printingwizard_base.ui \
	printingwizard.cpp printingwizard.h \
	printstyle.cpp printstyle.h \
	printprogress.cpp printprogress.h \
	detailledstyle.cpp detailledstyle.h \
	mikesstyle.cpp mikesstyle.h

preview_DATA = detailed-style.png mike-style.png

previewdir = $(kde_datadir)/kaddressbook/printing
