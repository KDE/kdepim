KDE_CXXFLAGS = $(LIBQPTHREAD_CXXFLAGS)

INCLUDES = -I$(top_srcdir)/rmm -I$(top_srcdir)/empath/lib $(all_includes)

lib_LTLIBRARIES = libempathui.la

libempathui_la_LDFLAGS = $(all_libraries)
libempathui_la_LIBADD = $(LIB_KIO) $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE)

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
EmpathHeaderSpecWidget.cpp \
EmpathHeaderViewWidget.cpp \
EmpathIdentitySettingsDialog.cpp \
EmpathListView.cpp \
EmpathMainWidget.cpp \
EmpathMainWindow.cpp \
EmpathMessageMarkDialog.cpp \
EmpathMatchPropertiesDialog.cpp \
EmpathMessageDownloadStatus.cpp \
EmpathMessageHTMLView.cpp \
EmpathMessageListItem.cpp \
EmpathMessageListWidget.cpp \
EmpathMessageStructureWidget.cpp \
EmpathMessageStructureItem.cpp \
EmpathMessageViewWidget.cpp \
EmpathMessageViewWindow.cpp \
EmpathMessageWidget.cpp \
EmpathPathSelectWidget.cpp \
EmpathPasswordEditWidget.cpp \
EmpathTaskWidget.cpp \
EmpathSubjectSpecWidget.cpp \
EmpathSendingSettingsDialog.cpp \
EmpathSetupWizard.cpp \
EmpathUI.cpp \
EmpathUIUtils.cpp

