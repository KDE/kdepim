INCLUDES = -I$(top_srcdir) -I$(top_builddir)/kaddressbook -I$(top_srcdir)/kaddressbook \
           -I$(top_srcdir)/kaddressbook/details -I$(top_srcdir)/kaddressbook/common $(all_includes) 

noinst_LTLIBRARIES = libprinter.la

libprinter_la_SOURCES = \
	detailledstyle.cpp  \
	ds_appearance.ui \
	kabentrypainter.cpp \
	mikesstyle.cpp \
	ringbinderstyle.cpp \
	printingwizard.cpp \
	printprogress.cpp \
	printstyle.cpp \
	rbs_appearance.ui \
	selectionpage.cpp \
	stylepage.cpp
libprinter_la_COMPILE_FIRST = $(top_builddir)/kaddressbook/kabprefs_base.h

EXTRA_DIST = \
	detailledstyle.cpp detailledstyle.h kabentrypainter.h \
	mikesstyle.cpp mikesstyle.h ringbinderstyle.h \
	printingwizard.cpp printingwizard.h \
	printprogress.cpp printprogress.h \
	printstyle.cpp printstyle.h rbs_appearance.ui.h \
	selectionpage.cpp selectionpage.h \
	stylepage.cpp stylepage.h

libprinter_la_METASOURCES = AUTO

previewdir = $(kde_datadir)/kaddressbook/printing
preview_DATA = detailed-style.png mike-style.png ringbinder-style.png
