
# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to. 
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# just set the variable
APPSDIR = $(kde_appsdir)/Utilities
# set the include path for X, qt and KDE
INCLUDES= @all_includes@ -I/usr/local/include
# claim, which subdirectories you want to install
SUBDIRS = doc

####### This part is very kpilot specific
# you can add here more. This one gets installed 
bin_PROGRAMS = 	kpilot kpilotDaemon pilotListener
noinst_LIBRARIES = libconduit.a

# KDE magic file
MAGIC=$(kde_mimedir)/magic

# Which sources should be compiled for kpilot
kpilot_SOURCES	= kpilot.cc kpilotlink.cc fileInstallWidget.cc pilotMemo.cc memoWidget.cc pilotComponent.cc \
	kpilotOptions.cc messageDialog.cc \
	pilotSerialDatabase.cc pilotLocalDatabase.cc pilotRecord.cc pilotAddress.cc \
	addressWidget.cc strToken.cc addressEditor.cpp addressEditorData.cpp pilotDateEntry.cc \
	logFile.cc conduitSetup.cc pilotTodoEntry.cc

kpilotDaemon_SOURCES = pilotDaemon.cc kpilotlink.cc pilotSerialDatabase.cc pilotLocalDatabase.cc pilotRecord.cc \
	messageDialog.cc logFile.cc

pilotListener_SOURCES = pilotListener.cc

libconduit_a_SOURCES = baseConduit.cc conduitApp.cc pilotRecord.cc pilotSerialDatabase.cc \
	messageDialog.cc pilotLocalDatabase.cc kpilotlink.cc

# the library search path
kpilot_LDFLAGS = @all_libraries@ -L/usr/local/lib
kpilotDaemon_LDFLAGS = @all_libraries@ -L/usr/local/lib

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
kpilot_LDADD   = -lkfile -lkfm -lkdeui -lkdecore -lqt -lX11 -lXext @LIBSOCKET@ -lpisock -lpicc
kpilotDaemon_LDADD = -lkfm -lkdeui -lkdecore -lqt -lX11 -lXext @LIBSOCKET@ -lpisock

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = kpilot.h kpilotlink.h fileInstallWidget.h pilotMemo.h memoWidget.h pilotComponent.h \
	baseConduit.h kpilotOptions.h messageDialog.h pilotDatabase.h \
	pilotSerialDatabase.h pilotLocalDatabase.h pilotRecord.h pilotAddress.h \
	addressWidget.h strToken.h pilotDaemon.h addressEditor.h pilotDateEntry.h \
	logFile.h conduitApp.h conduitSetup.h busysync.h hotsync.h kpilot_on_pp.h \
	pilotAppCategory.h pilotListEntry.h pilotUser.h statusMessages.h pilotTodoEntry.h

METASOURCES = AUTO

# make messages.po. Move this one to ../po/ and "make merge" in po
messages:
	$(XGETTEXT) -C -ki18n -kI18N_NOOP -ktranslate `find . -name "*.cc" -print` -x $(includedir)/kde.pot -o $(podir)/kpilot.pot

# just install datas here. Use install-exec-data for scripts and etc.
# the binary itself is already installed from automake
# use mkinstalldirs, not "install -d"
# don't install a list of file. Just one file per install.
# if you have more of them, create a subdirectory with an extra Makefile 
install-data-local: 
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) kpilot.kdelnk $(APPSDIR)
	$(INSTALL_DATA) KPilotDaemon.kdelnk $(APPSDIR)
	$(mkinstalldirs) $(kde_icondir)
	$(INSTALL_DATA) kpilot.xpm $(kde_icondir)
	$(INSTALL_DATA) kpilhck.xpm $(kde_icondir)
	$(INSTALL_DATA) kpillib.xpm $(kde_icondir)
	$(INSTALL_DATA) kpilpdb.xpm $(kde_icondir)
	$(INSTALL_DATA) kpilprc.xpm $(kde_icondir)
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-kpilot.xpm $(kde_minidir)/kpilot.xpm
	grep kpilot $(MAGIC) 2>&1 >/dev/null || { cat kpilot.magic >> $(MAGIC); }
	$(mkinstalldirs) $(kde_mimedir)/palmpilot
	$(INSTALL_DATA) application.kdelnk $(kde_mimedir)/palmpilot
	$(INSTALL_DATA) data.kdelnk $(kde_mimedir)/palmpilot
	$(INSTALL_DATA) hack.kdelnk $(kde_mimedir)/palmpilot
	$(INSTALL_DATA) library.kdelnk $(kde_mimedir)/palmpilot

# remove ALL you have installed in install-data-local or install-exec-local
uninstall-local:
	-rm -f $(APPSDIR)/kpilot.kdelnk
	-rm -f $(APPSDIR)/KPilotDaemon.kdelnk
	-rm -f $(kde_icondir)/kpilot.xpm
	-rm -f $(kde_icondir)/kpilhck.xpm
	-rm -f $(kde_icondir)/kpillib.xpm
	-rm -f $(kde_icondir)/kpilpdb.xpm
	-rm -f $(kde_icondir)/kpilprc.xpm
	-rm -f $(kde_minidir)/kpilot.xpm
	-rm -rf $(kde-mimedir)/palmpilot
	-sed -e'/start kpilot/,/stop kpilot/d' $(MAGIC)>$(MAGIC).new
	-cp $(MAGIC).new $(MAGIC)
	-rm -f $(MAGIC).new

