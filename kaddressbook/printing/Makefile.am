noinst_LTLIBRARIES = libprinter.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kaddressbook \
           -I$(top_srcdir)/kaddressbook/details $(all_includes) 

libprinter_la_METASOURCES = AUTO

libprinter_la_SOURCES = \
	detailledstyle.cpp  \
	ds_appearance.ui \
	mikesstyle.cpp \
	printingwizard.cpp \
	printprogress_base.ui printprogress.cpp \
	printstyle.cpp \
	selectionpage.cpp \
	stylepage.cpp

EXTRA_DIST = \
	detailledstyle.cpp detailledstyle.h \
	mikesstyle.cpp mikesstyle.h \
	printingwizard.cpp printingwizard.h \
	printprogress.cpp printprogress.h \
	printstyle.cpp printstyle.h \
	selectionpage.cpp selectionpage.h \
	stylepage.cpp stylepage.h

preview_DATA = detailed-style.png mike-style.png

previewdir = $(kde_datadir)/kaddressbook/printing
