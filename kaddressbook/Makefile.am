INCLUDES= -I$(top_srcdir)/kaddressbook/details -I$(top_srcdir)/kaddressbook/features\
	 -I$(top_srcdir)/kaddressbook/printing -I$(top_srcdir)/kaddressbook/views  \
	-I$(top_srcdir) -I$(top_builddir)/kaddressbook/printing \
	-I$(top_builddir)/kaddressbook/details $(all_includes) 

SUBDIRS = details features printing views . pics csv-templates

SRCDOC_DEST=$(kde_htmldir)/en/kaddressbook

bin_PROGRAMS = kaddressbook

kaddressbook_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kaddressbook_LDADD   = libkaddressbookpart.la
kaddressbook_SOURCES = main.cpp

kde_module_LTLIBRARIES = libkaddressbookpart.la

libkaddressbookpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkaddressbookpart_la_LIBADD  = $(top_builddir)/libkdepim/libkdepim.la \
	./details/libdetails.la \
	./features/libfeatures.la \
	./printing/libprinter.la \
	./views/libviews.la \
	$(LIB_KPARTS) $(LIB_KFILE) \
	-lkabc -lkdeprint

libkaddressbookpart_la_SOURCES = \
 actionmanager.cpp  addresseditwidget.cpp addresseeconfig.cpp \
 addresseeeditordialog.cpp addresseeeditorwidget.cpp addresseeutil.cpp \
 addviewdialog.cpp configureviewdialog.cpp configureviewfilterpage.cpp \
 emaileditwidget.cpp filter.cpp filtereditdialog.cpp \
 filterselectionwidget.cpp importdialog.cpp incsearchwidget.cpp \
 jumpbuttonbar.cpp kabprefs.cpp kaddressbook.cpp \
 kaddressbook_part.cpp kaddressbookiface.skel kaddressbookmain.cpp \
 kaddressbookprinter.cpp kaddressbookview.cpp \
 ldapsearchdialog.ui ldapsearchdialogimpl.cpp nameeditdialog.cpp \
 phoneeditwidget.cpp prefsdialog.cpp selectfieldswidget.cpp \
 undo.cpp undocmds.cpp viewmanager.cpp viewwrapper.cpp

kaddressbook_METASOURCES = AUTO

noinst_HEADERS = \
 actionmanager.h  addresseditwidget.h addresseeconfig.h \
 addresseeeditordialog.h addresseeeditorwidget.h addresseeutil.h \
 addviewdialog.h configureviewdialog.h configureviewfilterpage.h \
 emaileditwidget.h filter.h filtereditdialog.h \
 filterselectionwidget.h importdialog.h incsearchwidget.h \
 jumpbuttonbar.h kabprefs.h kaddressbook.h kaddressbook_part.h \
 kaddressbookmain.h kaddressbookprinter.h kaddressbookview.h \
 ldapsearchdialogimpl.h nameeditdialog.h phoneeditwidget.h prefsdialog.h \
 selectfieldswidget.h typecombo.h undo.h undocmds.h viewmanager.h \
 viewwrapper.h

rcdir = $(kde_datadir)/kaddressbook
rc_DATA = kaddressbookui.rc

partdir = $(kde_datadir)/kaddressbook
part_DATA = kaddressbook_part.rc

messages: rc.cpp
	$(EXTRACTRC) *.rc *.ui */*.ui >> rc.cpp
	$(XGETTEXT) *.h *.cpp */*.cpp */*.h -o $(podir)/kaddressbook.pot

srcdoc:
	$(mkinstalldirs) $(SRCDOC_DEST)
	kdoc -H -d $(SRCDOC_DEST) kaddressbook $(noinst_HEADERS) -lqt

appsdir = $(kde_appsdir)/Utilities
apps_DATA = kaddressbook.desktop

KDE_ICON = kaddressbook

include $(top_srcdir)/admin/Doxyfile.am
