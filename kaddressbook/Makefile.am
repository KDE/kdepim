INCLUDES= -I./printing -I$(top_srcdir) $(all_includes)

SUBDIRS = detailledview printing . pics

SRCDOC_DEST=$(kde_htmldir)/en/kaddressbook

bin_PROGRAMS = kaddressbook

kaddressbook_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kaddressbook_LDADD   = libkaddressbookpart.la
kaddressbook_SOURCES = main.cpp 

kde_module_LTLIBRARIES = libkaddressbookpart.la

libkaddressbookpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkaddressbookpart_la_LIBADD  = ./detailledview/libdetailledview.la \
				 ./printing/libprinter.la \
                                 $(top_builddir)/libkdepim/libkdepim.la \
				 $(LIB_KPARTS) $(LIB_KFILE) \
                                 -lkabc -lkdeprint
libkaddressbookpart_la_SOURCES = kaddressbook.cpp kaddressbookiface.skel \
  undo.cpp kaddressbookview.cpp kabprefs.cpp \
  undocmds.cpp contactlistview.cpp kimportdialog.cpp kldapclient.cpp \
  addhostdialog.ui KDListBoxPair.cpp selectfieldswidget.cpp \
  kaddressbook_part.cpp kaddressbookmain.cpp kaddressbooktableview.cpp \
  importdialog.cpp kaddressbookiconview.cpp configureviewdialog.cpp \
  kabentrypainter.cpp \
  ldapoptionswidget.ui ldapoptionswidgetimpl.cpp \
  ldapsearchdialog.ui ldapsearchdialogimpl.cpp \
  prefsdialog.cpp viewwrapper.cpp addviewdialog.cpp viewmanager.cpp \
  jumpbuttonbar.cpp configuretableviewdialog.cpp addresseeutil.cpp \
  cardview.cpp kaddressbookcardview.cpp addresseeeditorwidget.cpp \
  addresseeeditordialog.cpp configurecardviewdialog.cpp \
  addresseeeditorsupportwidgets.cpp kaddressbookprinter.cpp \
  filtereditdialog.cpp filtereditwidget.cpp filter.cpp \
  filterselectionwidget.cpp configureviewfilterpage.cpp \
  actionmanager.cpp

kaddressbook_METASOURCES = AUTO

noinst_HEADERS = kaddressbook.h undo.h kaddressbookview.h undocmds.h \
  contactlistview.h kimportdialog.h kldapclient.h kabprefs.h \
  KDListBoxPair.h kaddressbook_part.h selectfieldswidget.h \
  kaddressbookmain.h kaddressbooktableview.h importdialog.h \
  kaddressbookiconview.h configureviewdialog.h \
  ldapsearchdialogimpl.h ldapoptionswidgetimpl.h \
  prefsdialog.h viewwrapper.h addviewdialog.h viewmanager.h \
  jumpbuttonbar.h configuretableviewdialog.h addresseeutil.h \
  cardview.h kaddressbookcardview.h addresseeeditorwidget.h \
  addresseeeditordialog.h configurecardviewdialog.h \
  addresseeeditorsupportwidgets.h kaddressbookprinter.h \
  actionmanager.h \
  filtereditdialog.h filtereditwidget.h filter.h filterselectionwidget.h \
  configureviewfilterpage.h kabentrypainter.h

rcdir = $(kde_datadir)/kaddressbook
rc_DATA = kaddressbookui.rc

partdir   = $(kde_datadir)/kaddressbook
part_DATA = kaddressbook_part.rc

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kaddressbook.pot

srcdoc:
	$(mkinstalldirs) $(SRCDOC_DEST)
	kdoc -H -d $(SRCDOC_DEST) kaddressbook $(noinst_HEADERS) -lqt

appsdir = $(kde_appsdir)/Utilities
apps_DATA = kaddressbook.desktop

KDE_ICON = kaddressbook

kaddressbook.lo: printing/printingwizard_base.h
