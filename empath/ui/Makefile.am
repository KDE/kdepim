KDE_CXXFLAGS = $(LIBQPTHREAD_CXXFLAGS) -DQT_NO_ASCII_CAST

INCLUDES = \
	-I$(top_srcdir)/empath/parts/libEmpathMessageListWidget \
	-I$(top_srcdir)/empath/lib \
	-I$(top_srcdir)/rmm \
	$(all_includes)

lib_LTLIBRARIES = libempathui.la

libempathui_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module
#	Define this for testing
#	-no-undefined

rcdir = $(kde_datadir)/empath
rc_DATA = EmpathMainWindow.rc

libempathui_la_LIBADD = $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KIO) $(LIB_KDECORE) \
	$(top_builddir)/empath/parts/libEmpathMessageListWidget/libEmpathMessageListWidget.la \
	$(top_builddir)/empath/lib/libempathkernel.la

METASOURCES = AUTO

libempathui_la_SOURCES = \
EmpathAccountsSettingsDialog.cpp \
EmpathAddressSelectionWidget.cpp \
EmpathAttachmentEditDialog.cpp \
EmpathAttachmentListWidget.cpp \
EmpathAttachmentListItem.cpp \
EmpathAttachmentViewWidget.cpp \
EmpathComposeSettingsDialog.cpp \
EmpathComposeWidget.cpp \
EmpathComposeWindow.cpp \
EmpathConfigPOP3Dialog.cpp \
EmpathConfigPOP3Logging.cpp \
EmpathConfigIMAP4Dialog.cpp \
EmpathConfigMaildirDialog.cpp \
EmpathDisplaySettingsDialog.cpp \
EmpathEnvelopeWidget.cpp \
EmpathFilterActionDialog.cpp \
EmpathFilterEditDialog.cpp \
EmpathFilterListItem.cpp \
EmpathFilterManagerDialog.cpp \
EmpathFindDialog.cpp \
EmpathFolderChooserDialog.cpp \
EmpathFolderChooserWidget.cpp \
EmpathFolderListItem.cpp \
EmpathFolderWidget.cpp \
EmpathHeaderBodyWidget.cpp \
EmpathHeaderSpecWidget.cpp \
EmpathHeaderViewWidget.cpp \
EmpathIdentitySettingsDialog.cpp \
EmpathListView.cpp \
EmpathMainWidget.cpp \
EmpathMainWindow.cpp \
EmpathMatchPropertiesDialog.cpp \
EmpathMessageDownloadStatus.cpp \
EmpathMessageHTMLView.cpp \
EmpathMessageStructureWidget.cpp \
EmpathMessageStructureItem.cpp \
EmpathMessageViewWidget.cpp \
EmpathMessageViewWindow.cpp \
EmpathMessageWidget.cpp \
EmpathPathSelectWidget.cpp \
EmpathPasswordEditWidget.cpp \
EmpathProgressIndicator.cpp \
EmpathTaskWidget.cpp \
EmpathTextSelectionWidget.cpp \
EmpathSubjectSpecWidget.cpp \
EmpathSendingSettingsDialog.cpp \
EmpathSetupWizard.cpp \
EmpathUI.cpp \
EmpathUIUtils.cpp

