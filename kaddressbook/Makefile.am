SUBDIRS = interfaces common printing editors . views features kcmconfigs xxport pics csv-templates \
          thumbnailcreator
SRCDOC_DEST = $(kde_htmldir)/en/kaddressbook


INCLUDES= -I$(top_srcdir)/kaddressbook/interfaces \
          -I$(top_srcdir)/kaddressbook/features \
          -I$(top_srcdir)/kaddressbook/views \
          -I$(top_srcdir)/kaddressbook/printing \
          -I$(top_srcdir)/kaddressbook/common \
          -I$(top_srcdir)/kaddressbook/editors \
          -I$(top_builddir)/kaddressbook/common \
          -I$(top_srcdir) \
	  -I$(top_srcdir)/libkdepim \
	  -I$(top_builddir)/libkdepim \
          -I$(top_buildir)/kaddressbook/editors \
          -I$(top_srcdir)/akregator/src \
          $(all_includes)

bin_PROGRAMS = kaddressbook

kaddressbook_SOURCES = main.cpp kaddressbookmain.cpp 
kaddressbook_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kaddressbook_LDADD = libkaddressbook.la

kde_module_LTLIBRARIES = libkaddressbookpart.la

libkaddressbookpart_la_SOURCES = kaddressbook_part.cpp
libkaddressbookpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
libkaddressbookpart_la_LIBADD = libkaddressbook.la

lib_LTLIBRARIES = libkaddressbook.la
libkaddressbook_la_SOURCES = addresseeconfig.cpp addresseeeditordialog.cpp \
    addresseeeditorwidget.cpp addresseeutil.cpp addresseditwidget.cpp \
    addviewdialog.cpp emaileditwidget.cpp extensionmanager.cpp \
    filtereditdialog.cpp filterselectionwidget.cpp geowidget.cpp \
    imagewidget.cpp incsearchwidget.cpp jumpbuttonbar.cpp kabcore.cpp kablock.cpp \
    kaddressbookiface.skel kaddressbookservice.cpp \
    kaddressbookview.cpp keywidget.cpp ldapsearchdialog.cpp nameeditdialog.cpp \
    phoneeditwidget.cpp secrecywidget.cpp soundwidget.cpp \
    viewconfigurefieldspage.cpp viewconfigurefilterpage.cpp \
    viewconfigurewidget.cpp undo.cpp undocmds.cpp viewmanager.cpp \
    xxportmanager.cpp xxportselectdialog.cpp contacteditorwidgetmanager.cpp \
    simpleaddresseeeditor.cpp advancedcustomfields.cpp customfieldswidget.cpp \
    freebusywidget.cpp searchmanager.cpp imeditwidget.cpp
libkaddressbook_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined
libkaddressbook_la_LIBADD = $(top_builddir)/kaddressbook/printing/libprinter.la \
                            $(top_builddir)/kaddressbook/interfaces/libkabinterfaces.la \
                            $(top_builddir)/libkdepim/libkdepim.la \
                            $(top_builddir)/libkcal/libkcal.la \
                            -lkabc_file $(LIB_KABC) $(LIB_KHTML) \
                            $(top_builddir)/kaddressbook/common/libkabcommon.la \
                            $(top_builddir)/kaddressbook/editors/libkabim.la \
                            ../akregator/src/librss/librsslocal.la \
                            -lkdeprint
libkaddressbook_la_COMPILE_FIRST = $(top_builddir)/libkdepim/categoryselectdialog.h \
                                   $(top_builddir)/libkdepim/categoryeditdialog.h \
                                   $(top_builddir)/kaddressbook/common/kabprefs_base.h


noinst_HEADERS = \
    addresseditwidget.h addresseeconfig.h \
    addresseeeditordialog.h addresseeeditorwidget.h addresseeutil.h \
    addviewdialog.h emaileditwidget.h extensionmanager.h \
    filtereditdialog.h filterselectionwidget.h geowidget.h imagewidget.h \
    incsearchwidget.h jumpbuttonbar.h kablock.h kaddressbook_part.h \
    kaddressbookservice.h kaddressbookview.h keywidget.h \
    ldapsearchdialog.h nameeditdialog.h phoneeditwidget.h secrecywidget.h \
    soundwidget.h typecombo.h undo.h undocmds.h viewconfigurefieldspage.h \
    viewconfigurefilterpage.h viewconfigurewidget.h viewmanager.h xxportmanager.h \
    xxportselectdialog.h advancedcustomfields.h imeditwidget.h

kaddressbook_METASOURCES = AUTO

rcdir = $(kde_datadir)/kaddressbook
rc_DATA = kaddressbookui.rc kaddressbook_part.rc

kabdatadir = $(kde_datadir)/kaddressbook
kabdata_DATA = zone.tab

xdg_apps_DATA = kaddressbook.desktop

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kaddressbook_view.desktop dcopaddressbook.desktop

KDE_ICON = kaddressbook

messages: rc.cpp
	$(EXTRACTRC) */*.ui >> rc.cpp
	$(XGETTEXT) *.h *.cpp common/*.cpp editors/*.cpp features/*.cpp printing/*.cpp views/*.cpp xxport/*.cpp interfaces/*.h -o $(podir)/kaddressbook.pot

srcdoc:
	$(mkinstalldirs) $(SRCDOC_DEST)
	kdoc -H -d $(SRCDOC_DEST) kaddressbook $(noinst_HEADERS) -lqt

include ../admin/Doxyfile.am
