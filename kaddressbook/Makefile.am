INCLUDES= -I$(top_srcdir)/kaddressbook/details -I$(top_srcdir)/kaddressbook/features \
	  -I$(top_srcdir)/kaddressbook/printing -I$(top_srcdir)/kaddressbook/views \
	  -I$(top_srcdir) -I$(top_builddir)/kaddressbook/printing \
	  -I$(top_builddir)/kaddressbook/details $(all_includes) 

SUBDIRS = details printing . views features kcmconfigs xxport pics csv-templates

SRCDOC_DEST=$(kde_htmldir)/en/kaddressbook

bin_PROGRAMS = kaddressbook

kaddressbook_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kaddressbook_LDADD   = libkaddressbook.la
kaddressbook_SOURCES = main.cpp

kde_module_LTLIBRARIES = libkaddressbookpart.la

libkaddressbookpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkaddressbookpart_la_LIBADD  = libkaddressbook.la
libkaddressbookpart_la_SOURCES = kaddressbook_part.cpp

kaddressbook_METASOURCES = AUTO

noinst_LTLIBRARIES = libkabcommon.la
libkabcommon_la_SOURCES = kabprefs.cpp configurewidget.cpp

lib_LTLIBRARIES = libkaddressbook.la

libkaddressbook_la_SOURCES = xxportobject.cpp xxportmanager.cpp kabcore.cpp \
 undo.cpp undocmds.cpp viewmanager.cpp filter.cpp filtereditdialog.cpp \
 xxportselectdialog.cpp extensionmanager.cpp incsearchwidget.cpp \
 addresseeutil.cpp \
 jumpbuttonbar.cpp addresseditwidget.cpp addresseeconfig.cpp \
 addresseeeditordialog.cpp addresseeeditorwidget.cpp \
 addviewdialog.cpp emaileditwidget.cpp extensionwidget.cpp \
 filterselectionwidget.cpp geowidget.cpp imagewidget.cpp \
 kaddressbookiface.skel kaddressbookmain.cpp \
 kaddressbookview.cpp keywidget.cpp ldapsearchdialog.cpp nameeditdialog.cpp \
 phoneeditwidget.cpp secrecywidget.cpp soundwidget.cpp \
 viewconfigurefieldspage.cpp viewconfigurefilterpage.cpp \
 viewconfigurewidget.cpp
libkaddressbook_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined
libkaddressbook_la_LIBADD = details/libdetails.la printing/libprinter.la \
        libkabcommon.la $(top_builddir)/libkdepim/libkdepim.la \
	$(LIB_KABC) $(LIB_KPARTS) $(LIB_KHTML) -lkdeprint

kaddressbookincludedir = $(includedir)/kaddressbook
kaddressbookinclude_HEADERS = extensionwidget.h kabcore.h xxportobject.h

noinst_HEADERS = \
 addresseditwidget.h addresseeconfig.h \
 addresseeeditordialog.h addresseeeditorwidget.h addresseeutil.h \
 addviewdialog.h configurewidget.h emaileditwidget.h extensionmanager.h filter.h \
 filtereditdialog.h filterselectionwidget.h geowidget.h imagewidget.h \
 incsearchwidget.h jumpbuttonbar.h kabprefs.h \
 kaddressbook_part.h kaddressbookmain.h kaddressbookview.h keywidget.h \
 ldapsearchdialog.h nameeditdialog.h phoneeditwidget.h secrecywidget.h \
 soundwidget.h typecombo.h undo.h undocmds.h viewconfigurefieldspage.h \
 viewconfigurefilterpage.h viewconfigurewidget.h viewmanager.h xxportmanager.h \
 xxportselectdialog.h

rcdir = $(kde_datadir)/kaddressbook
rc_DATA = kaddressbookui.rc

partdir = $(kde_datadir)/kaddressbook
part_DATA = kaddressbook_part.rc

kabdata_DATA = zone.tab
kabdatadir = $(kde_datadir)/kaddressbook

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
servicetype_DATA = kaddressbook_extension.desktop kaddressbook_view.desktop \
 kaddressbook_xxport.desktop dcopaddressbook.desktop

include ../admin/Doxyfile.am
