
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

# just to make sure, automake makes them 
BUILTSOURCES =	kpilot.moc kpilotlink.moc fileInstallWidget.moc pilotComponent.moc \
	kpilotOptions.moc messageDialog.moc \
	addressWidget.moc pilotDaemon.moc addressEditor.moc conduitSetup.moc pilotTodoEntry.moc

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(BUILTSOURCES)

# make messages.po. Move this one to ../po/ and "make merge" in po
messages:
	$(XGETTEXT) -C -ktranslate $(kpilot_SOURCES)

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

# add a dependency for every moc file to be full portable
# I've added a key binding to emacs for this. 
kpilot.cc:kpilot.moc
kpilot.moc: kpilot.h
	$(MOC) kpilot.h -o kpilot.moc
kpilotlink.cc:kpilotlink.moc
kpilotlink.moc: kpilotlink.h
	$(MOC) kpilotlink.h -o kpilotlink.moc
fileInstallWidget.cc: fileInstallWidget.moc
fileInstallWidget.moc: fileInstallWidget.h
	$(MOC) fileInstallWidget.h -o fileInstallWidget.moc
memoWidget.cc: memoWidget.moc 
memoWidget.moc: memoWidget.h
	$(MOC) memoWidget.h -o memoWidget.moc
pilotComponent.cc: pilotComponent.moc
pilotComponent.moc: pilotComponent.h
	$(MOC) pilotComponent.h -o pilotComponent.moc
kpilotOptions.cc: kpilotOptions.moc
kpilotOptions.moc: kpilotOptions.h
	$(MOC) kpilotOptions.h -o kpilotOptions.moc
passworddialog.cc: passworddialog.moc
passworddialog.moc: passworddialog.h
	$(MOC) passworddialog.h -o passworddialog.moc
messageDialog.cc: messageDialog.moc
messageDialog.moc: messageDialog.h
	$(MOC) messageDialog.h -o messageDialog.moc
addressWidget.cc: addressWidget.moc
addressWidget.moc: addressWidget.h
	$(MOC) addressWidget.h -o addressWidget.moc
pilotDaemon.cc: pilotDaemon.moc
pilotDaemon.moc: pilotDaemon.h
	$(MOC) pilotDaemon.h -o pilotDaemon.moc
addressEditorData.cpp: addressEditor.moc
addressEditor.moc: addressEditor.h
	$(MOC) addressEditor.h -o addressEditor.moc
conduitSetup.cc: conduitSetup.moc
conduitSetup.moc: conduitSetup.h
	$(MOC) conduitSetup.h -o conduitSetup.moc
baseConduit.cc: baseConduit.moc
baseConduit.moc: baseConduit.h
	$(MOC) baseConduit.h -o baseConduit.moc
conduitApp.cc: conduitApp.moc
conduitApp.moc: conduitApp.h
	$(MOC) conduitApp.h -o conduitApp.moc
