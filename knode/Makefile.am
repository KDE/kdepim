INCLUDES= $(all_includes) -DQT_NO_ASCII_CAST -DQT_NO_COMPAT -DQT_CLEAN_NAMESPACE
SUBDIRS = toolbar pics filters

bin_PROGRAMS = knode
knode_SOURCES = kngroupbrowser.cpp knpurgeprogressdialog.cpp knsmtpclient.cpp \
	knnntpclient.cpp knprotocolclient.cpp \
	knaccountmanager.cpp knnntpaccount.cpp knserverinfo.cpp \
	kncollection.cpp knviewheader.cpp knreadhdrsettings.cpp \
	knfiltersettings.cpp knpostcomsettings.cpp knposttechsettings.cpp \
	knreadgensettings.cpp knreadappsettings.cpp kncleanupsettings.cpp \
	knaccmailsettings.cpp knaccnewssettings.cpp knusersettings.cpp \
	knkeysettings.cpp knsettingswidget.cpp knsettingsdialog.cpp \
	knsearchdialog.cpp knfilterconfigwidget.cpp knsenderrordialog.cpp \
	kngroupselectdialog.cpp knsavedarticlemanager.cpp knfetcharticlemanager.cpp \
	knfoldermanager.cpp kncleanup.cpp knsavedarticle.cpp kncomposer.cpp \
	knfolder.cpp kncollectionviewitem.cpp knarticlewindow.cpp \
	knarticlemanager.cpp knarticlefilter.cpp kngroupmanager.cpp \
	knjobdata.cpp knnetaccess.cpp kngroup.cpp knarticlecollection.cpp \
	knfetcharticle.cpp knarticle.cpp knmimeinfo.cpp knmimecontent.cpp \
	knarticlebase.cpp knfocuswidget.cpp knarticlewidget.cpp \
	kngrouppropdlg.cpp knuserwidget.cpp knuserentry.cpp \
	knscoredialog.cpp knthread.cpp convert.cpp knfiltermanager.cpp \
	knstatusfilter.cpp knstringfilter.cpp knrangefilter.cpp \
	knfilterdialog.cpp kngroupdialog.cpp  knstringsplitter.cpp \
        knhdrviewitem.cpp knlistview.cpp knlistbox.cpp 	utilities.cpp \
        knodeview.cpp knode.cpp main.cpp

noinst_HEADERS = knaccmailsettings.h      knglobals.h              knrangefilter.h \
	knaccnewssettings.h      kngroup.h                knreadappsettings.h \
	knaccountmanager.h       kngroupdialog.h          knreadgensettings.h \
	knarticle.h              knreadhdrsettings.h \
	knarticlebase.h          kngroupmanager.h         knsavedarticle.h \
	knarticlecollection.h    kngrouppropdlg.h         knsavedarticlemanager.h \
	knarticlefilter.h        kngroupselectdialog.h    knscoredialog.h \
	knarticlemanager.h       knhdrviewitem.h          knsearchdialog.h \
	knarticlewidget.h        knjobdata.h              knsenderrordialog.h \
	knarticlewindow.h        knkeysettings.h          knserverinfo.h \
	kncleanup.h              knsettingsdialog.h \
	kncleanupsettings.h      knlistbox.h              knsettingswidget.h \
	kncollection.h           knlistview.h             knsmtpclient.h \
	kncollectionviewitem.h   knstatusfilter.h \
	kncomposer.h             knmimecontent.h          knstringfilter.h \
	knmimeinfo.h             knstringsplitter.h \
	knfetcharticle.h         knnetaccess.h            knthread.h \
	knfetcharticlemanager.h  knnntpaccount.h          knuserentry.h \
	knfilterconfigwidget.h   knnntpclient.h           knusersettings.h \
	knfilterdialog.h         knode.h                  knuserwidget.h \
	knfiltermanager.h        knodeview.h              knviewheader.h \
	knfiltersettings.h       knpostcomsettings.h \
	knfocuswidget.h          knposttechsettings.h     resource.h \
	knfolder.h               knprotocolclient.h       utilities.h \
	knfoldermanager.h        knpurgeprogressdialog.h  kngroupbrowser.h

knode_LDFLAGS = $(all_libraries) $(KDE_RPATH) 
knode_LDADD   = $(LIB_KHTML) -lkspell -lmimelib $(LIB_KAB) $(LIB_KFILE) -lpthread $(LIBRESOLV)
knode_METASOURCES = AUTO

appsdir = $(kde_appsdir)/Internet
apps_DATA = KNode.desktop

rcdir = $(kde_datadir)/knode
rc_DATA = headers.rc knodeui.rc knreaderui.rc kncomposerui.rc

KDE_ICON = AUTO

EXTRA_DIST = KNode.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/knode.pot
