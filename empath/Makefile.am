APPSDIR = $(kde_appsdir)/Internet
INCLUDES= $(all_includes) -I rmm/include/
SUBDIRS = doc pics tips rmm templates

bin_PROGRAMS = 	empath

empath_LDADD   = rmm/.libs/librmm.a -lkhtml -lkimgio -ljpeg6b -lX11 -lm -ljscript -lkdeutil -lkfile -lkfm -lkdeui -lkdecore -lqt -lXext -lmimelib -lpng -ltiff -lz -ldl

empath_LDFLAGS = $(all_libraries) $(KDE_RPATH)

empath_METASOURCES = USE_AUTOMOC

empath_SOURCES = \
	Empath.cpp \
	EmpathAboutBox.cpp \
	EmpathAccountsSettingsDialog.cpp \
	EmpathAddressSelectionWidget.cpp \
	EmpathAttachmentListWidget.cpp \
	EmpathComposeSettingsDialog.cpp \
	EmpathComposeWidget.cpp \
	EmpathComposeWindow.cpp \
	EmpathConfigIMAP4Dialog.cpp \
	EmpathConfigMaildirDialog.cpp \
	EmpathConfigMboxDialog.cpp \
	EmpathConfigMMDFDialog.cpp \
	EmpathConfigPOP3Dialog.cpp \
	EmpathConfigPOP3General.cpp \
	EmpathConfigPOP3LargeMsgs.cpp \
	EmpathConfigPOP3Logging.cpp \
	EmpathConfigPOP3Password.cpp \
	EmpathConfigPOP3Server.cpp \
	EmpathConfigPOP3Widget.cpp \
	EmpathDisplaySettingsDialog.cpp \
	EmpathFilter.cpp \
	EmpathFilterActionDialog.cpp \
	EmpathFilterEditDialog.cpp \
	EmpathFilterEventHandler.cpp \
	EmpathFilterList.cpp \
	EmpathFilterListItem.cpp \
	EmpathFilterManagerDialog.cpp \
	EmpathFindDialog.cpp \
	EmpathFolder.cpp \
	EmpathFolderChooserDialog.cpp \
	EmpathFolderChooserWidget.cpp \
	EmpathFolderList.cpp \
	EmpathFolderListItem.cpp \
	EmpathFolderWidget.cpp \
	EmpathHeaderBodyWidget.cpp \
	EmpathHeaderEditWidget.cpp \
	EmpathHeaderNameWidget.cpp \
	EmpathHeaderSpecWidget.cpp \
	EmpathIdentitySettingsDialog.cpp \
	EmpathMailSender.cpp \
	EmpathMailSenderQMTP.cpp \
	EmpathMailSenderQmail.cpp \
	EmpathMailSenderSMTP.cpp \
	EmpathMailSenderSendmail.cpp \
	EmpathMailbox.cpp \
	EmpathMailboxIMAP4.cpp \
	EmpathMailboxList.cpp \
	EmpathMailboxMMDF.cpp \
	EmpathMailboxMaildir.cpp \
	EmpathMailboxMbox.cpp \
	EmpathMailboxNameDialog.cpp \
	EmpathMailboxPOP3.cpp \
	EmpathMainWidget.cpp \
	EmpathMainWindow.cpp \
	EmpathMatchPropertiesDialog.cpp \
	EmpathMatcher.cpp \
	EmpathMenuMaker.cpp \
	EmpathMessage.cpp \
	EmpathMessageDataCache.cpp \
	EmpathIndexRecord.cpp \
	EmpathIndex.cpp \
	EmpathMessageDownloadStatus.cpp \
	EmpathMessageHTMLView.cpp \
	EmpathMessageList.cpp \
	EmpathMessageListItem.cpp \
	EmpathMessageListWidget.cpp \
	EmpathMessageSourceView.cpp \
	EmpathMessageViewWidget.cpp \
	EmpathMessageViewWindow.cpp \
	EmpathMessageWidget.cpp \
	EmpathNewsSettingsDialog.cpp \
	EmpathSendingSettingsDialog.cpp \
	EmpathServerSettingsDialog.cpp \
	EmpathServerTypeDialog.cpp \
	EmpathSettingsDialog.cpp \
	EmpathSettingsWidget.cpp \
	EmpathStatusWidget.cpp \
	EmpathSubjectSpecWidget.cpp \
	EmpathTimeStamp.cpp \
	EmpathTipOfTheDay.cpp \
	EmpathTaskWidget.cpp \
	EmpathURL.cpp \
	EmpathUtilities.cpp \
	EmpathDebugDialog.cpp \
	RikGroupBox.cpp \
	main.cpp \
	md5.cpp
#	LeakTracer.cpp

noinst_HEADERS = \
	Empath.h \
	EmpathAboutBox.h \
	EmpathAccountsSettingsDialog.h \
	EmpathAddressSelectionWidget.h \
	EmpathAttachmentListWidget.h \
	EmpathComposeSettingsDialog.h \
	EmpathComposeWidget.h \
	EmpathComposeWindow.h \
	EmpathConfigIMAP4Dialog.h \
	EmpathConfigMaildirDialog.h \
	EmpathConfigMboxDialog.h \
	EmpathConfigMMDFDialog.h \
	EmpathConfigPOP3Dialog.h \
	EmpathConfigPOP3General.h \
	EmpathConfigPOP3LargeMsgs.h \
	EmpathConfigPOP3Logging.h \
	EmpathConfigPOP3Password.h \
	EmpathConfigPOP3Server.h \
	EmpathConfigPOP3Widget.h \
	EmpathDefines.h \
	EmpathDisplaySettingsDialog.h \
	EmpathEnum.h \
	EmpathFilter.h \
	EmpathFilterActionDialog.h \
	EmpathFilterEditDialog.h \
	EmpathFilterEventHandler.h \
	EmpathFilterList.h \
	EmpathFilterListItem.h \
	EmpathFilterManagerDialog.h \
	EmpathFindDialog.h \
	EmpathFolder.h \
	EmpathFolderChooserDialog.h \
	EmpathFolderChooserWidget.h \
	EmpathFolderList.h \
	EmpathFolderListItem.h \
	EmpathFolderWidget.h \
	EmpathHeaderBodyWidget.h \
	EmpathHeaderEditWidget.h \
	EmpathHeaderNameWidget.h \
	EmpathHeaderSpecWidget.h \
	EmpathIdentitySettingsDialog.h \
	EmpathMailSender.h \
	EmpathMailSenderQMTP.h \
	EmpathMailSenderQmail.h \
	EmpathMailSenderSMTP.h \
	EmpathMailSenderSendmail.h \
	EmpathMailbox.h \
	EmpathMailboxIMAP4.h \
	EmpathMailboxList.h \
	EmpathMailboxMMDF.h \
	EmpathMailboxMaildir.h \
	EmpathMailboxMbox.h \
	EmpathMailboxNameDialog.h \
	EmpathMailboxPOP3.h \
	EmpathMainWidget.h \
	EmpathMainWindow.h \
	EmpathMatchPropertiesDialog.h \
	EmpathMatcher.h \
	EmpathMenuMaker.h \
	EmpathMessage.h \
	EmpathMessageDataCache.h \
	EmpathIndexRecord.h \
	EmpathIndex.h \
	EmpathMessageDownloadStatus.h \
	EmpathMessageHTMLView.h \
	EmpathMessageList.h \
	EmpathMessageListItem.h \
	EmpathMessageListWidget.h \
	EmpathMessageSourceView.h \
	EmpathMessageViewWidget.h \
	EmpathMessageViewWindow.h \
	EmpathMessageWidget.h \
	EmpathNewsSettingsDialog.h \
	EmpathSendingSettingsDialog.h \
	EmpathServerSettingsDialog.h \
	EmpathServerTypeDialog.h \
	EmpathSettingsDialog.h \
	EmpathSettingsWidget.h \
	EmpathStatusWidget.h \
	EmpathSubjectSpecWidget.h \
	EmpathTimeStamp.h \
	EmpathTipOfTheDay.h \
	EmpathTaskWidget.h \
	EmpathURL.h \
	EmpathUtilities.h \
	EmpathDebugDialog.h \
	RikGroupBox.h \
	empathConfigDefaults.h \
	md5.h

	DISTCLEANFILES = $(empath_METASOURCES)

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(empath_SOURCES) && mv messages.po ../po/empath.pot

install-data-local: 
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) empath.kdelnk $(APPSDIR)
	$(mkinstalldirs) $(kde_datadir)/empath
	
uninstall-local:
	-rm -f $(APPSDIR)/empath.kdelnk
	-rm -rf $(kde_datadir)/empath

