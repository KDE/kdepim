INCLUDES= -I$(top_srcdir)/kaddressbook/details -I$(top_srcdir)/kaddressbook/features\
	 -I$(top_srcdir)/kaddressbook/printing -I$(top_srcdir)/kaddressbook/views  \
	-I$(top_srcdir) -I$(top_builddir)/kaddressbook/printing \
	-I$(top_builddir)/kaddressbook/details $(all_includes) 

SUBDIRS = details printing . views features pics csv-templates

SRCDOC_DEST=$(kde_htmldir)/en/kaddressbook

bin_PROGRAMS = kaddressbook

kaddressbook_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kaddressbook_LDADD   = libkaddressbookpart.la
kaddressbook_SOURCES = main.cpp

kde_module_LTLIBRARIES = libkaddressbookpart.la

libkaddressbookpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkaddressbookpart_la_LIBADD  = $(top_builddir)/libkdepim/libkdepim.la \
	./details/libdetails.la \
	./printing/libprinter.la \
	$(LIB_KPARTS) $(LIB_KHTML) \
	-lkabc -lkdeprint

libkaddressbookpart_la_SOURCES = \
 actionmanager.cpp addhostdialog.cpp addresseditwidget.cpp addresseeconfig.cpp \
 addresseeeditordialog.cpp addresseeeditorwidget.cpp addresseeutil.cpp \
 addviewdialog.cpp configurewidget.cpp \
 emaileditwidget.cpp extensionwidget.cpp filter.cpp filtereditdialog.cpp \
 filterselectionwidget.cpp geowidget.cpp importdialog.cpp incsearchwidget.cpp \
 jumpbuttonbar.cpp kabprefs.cpp kaddressbook.cpp \
 kaddressbook_part.cpp kaddressbookiface.skel kaddressbookmain.cpp \
 kaddressbookview.cpp ldapoptionswidget.cpp \
 ldapsearchdialog.ui ldapsearchdialogimpl.cpp nameeditdialog.cpp \
 phoneeditwidget.cpp prefsdialog.cpp undo.cpp undocmds.cpp \
 viewconfigurefieldspage.cpp viewconfigurefilterpage.cpp \
 viewconfigurewidget.cpp viewmanager.cpp

kaddressbook_METASOURCES = AUTO

noinst_HEADERS = \
 actionmanager.h addhostdialog.h addresseditwidget.h addresseeconfig.h \
 addresseeeditordialog.h addresseeeditorwidget.h addresseeutil.h \
 addviewdialog.h configurewidget.h \
 emaileditwidget.h filter.h filtereditdialog.h filterselectionwidget.h \
 geowidget.h importdialog.h incsearchwidget.h \
 jumpbuttonbar.h kabprefs.h kaddressbook.h kaddressbook_part.h \
 kaddressbookmain.h kaddressbookview.h ldapoptionswidget.h \
 ldapsearchdialogimpl.h nameeditdialog.h phoneeditwidget.h prefsdialog.h \
 typecombo.h undo.h undocmds.h viewconfigurefieldspage.h \
 viewconfigurefilterpage.h viewconfigurewidget.h viewmanager.h

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

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kaddressbook_extension.desktop kaddressbook_view.desktop dcopaddressbook.desktop

include ../admin/Doxyfile.am
