KDE_CXXFLAGS = $(USE_THREADS)

INCLUDES=	-I$(top_srcdir)/libkdenetwork \
		-I$(top_srcdir)/libkdepim \
		 $(all_includes)

noinst_LTLIBRARIES = libknodecommon.la

kde_module_LTLIBRARIES = libknodepart.la
libknodepart_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 1:0
libknodepart_la_LIBADD	= $(LIB_KHTML) $(LIB_KSPELL) $(LIB_KABC) -lkwalletclient \
			../libkdenetwork/libkdenetwork.la ../libkdepim/libkdepim.la \
			../mimelib/libmimelib.la libknodecommon.la
SUBDIRS = pics filters

bin_PROGRAMS = knode

knode_LDFLAGS = $(all_libraries) $(KDE_RPATH)
knode_LDADD   = -lkdeprint -lkwalletclient $(top_builddir)/libkdenetwork/libkdenetwork.la ../libkdepim/libkdepim.la libknodecommon.la $(LIB_KSPELL) $(LIB_KABC) $(LIB_KFILE) $(LIBRESOLV)


libknodecommon_la_SOURCES =   knconfigmanager.cpp \
                  knconfig.cpp \
                  knconfigwidgets.cpp \
                  \
                  knnetaccess.cpp \
                  knprotocolclient.cpp \
                  knsmtpclient.cpp \
                  knnntpclient.cpp \
                  knjobdata.cpp \
                  \
                  knaccountmanager.cpp \
                  kncollection.cpp \
                  kncollectionviewitem.cpp \
                  knserverinfo.cpp \
                  knnntpaccount.cpp \
                  \
                  kngroupmanager.cpp \
                  knarticlecollection.cpp \
                  kngroup.cpp \
                  kngroupbrowser.cpp \
                  kngroupselectdialog.cpp \
                  kngroupdialog.cpp \
                  kngrouppropdlg.cpp \
                  \
                  knfoldermanager.cpp \
                  knfolder.cpp \
                  \
                  knmemorymanager.cpp \
                  kncleanup.cpp \
                  knconvert.cpp \
                  \
                  knarticlemanager.cpp \
                  knarticle.cpp \
                  knarticlewidget.cpp \
                  knarticlewindow.cpp \
                  knhdrviewitem.cpp \
                  \
                  kncomposer.cpp \
                  knarticlefactory.cpp \
                  \
                  knfiltermanager.cpp \
                  knstatusfilter.cpp \
                  knstringfilter.cpp \
                  knrangefilter.cpp \
                  knarticlefilter.cpp \
                  knfilterconfigwidget.cpp \
                  knfilterdialog.cpp \
                  knsearchdialog.cpp \
                  \
                  utilities.cpp \
                  \
                  knscoring.cpp \
                  \
                  knwidgets.cpp \
                  knlistview.cpp \
		  knmainwidget.cpp    \
		  aboutdata.cpp \
		  knglobals.cpp \
		  knodecomposeriface.skel \
		  knodeiface.skel

knode_SOURCES = knode.cpp knapplication.cpp main.cpp

libknodepart_la_SOURCES = knode_part.cpp


noinst_HEADERS =  knconfigmanager.h \
                  knconfig.h \
                  \
                  knnetaccess.h \
                  knprotocolclient.h \
                  knsmtpclient.h \
                  knnntpclient.h \
                  knjobdata.h \
                  \
                  knaccountmanager.h \
                  kncollection.h \
                  kncollectionviewitem.h \
                  knserverinfo.h \
                  knnntpaccount.h \
                  \
                  kngroupmanager.h \
                  knarticlecollection.h \
                  kngroup.h \
                  kngroupbrowser.h \
                  kngroupselectdialog.h \
                  kngroupdialog.h \
                  kngrouppropdlg.h \
                  \
                  knfoldermanager.h \
                  knfolder.h \
                  \
                  knmemorymanager.h \
                  kncleanup.h \
                  knconvert.h \
                  \
                  knarticlemanager.h \
                  knarticle.h \
                  knarticlewidget.h \
                  knarticlewindow.h \
                  knhdrviewitem.h \
                  \
                  kncomposer.h \
                  knarticlefactory.h \
                  \
                  knfiltermanager.h \
                  knstatusfilter.h \
                  knstringfilter.h \
                  knrangefilter.h \
                  knarticlefilter.h \
                  knfilterconfigwidget.h \
                  knfilterdialog.h \
                  knsearchdialog.h \
                  \
                  utilities.h \
                  \
                  knscoring.h \
                  \
                  knwidgets.h \
                  knlistview.h \
                  knode.h \
                  knapplication.h \
                  resource.h \
                  knglobals.h \
		  aboutdata.h \
		  knmainwidget.h 


knode_METASOURCES = AUTO

appsdir = $(kde_appsdir)/Internet
apps_DATA = KNode.desktop

rcdir = $(kde_datadir)/knode
rc_DATA = headers.rc knodeui.rc knreaderui.rc kncomposerui.rc

knewsprotocol_DATA = knewsservice.protocol
knewsprotocoldir = $(kde_servicesdir)

KDE_ICON = AUTO

EXTRA_DIST = KNode.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/knode.pot
