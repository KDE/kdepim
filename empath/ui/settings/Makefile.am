KDE_CXXFLAGS = -DQT_NO_ASCII_CAST

INCLUDES = $(all_includes) -I$(top_srcdir)/empath/lib \
	-I$(top_srcdir)/librmm

lib_LTLIBRARIES = libEmpathSettingsDialog.la

libEmpathSettingsDialog_la_LDFLAGS = \
	$(all_libraries) -version-info 1:0:0 -module

libEmpathSettingsDialog_la_LIBADD = $(LIB_KDEUI)

METASOURCES = AUTO

libEmpathSettingsDialog_la_SOURCES = \
AccountsSettingsForm.ui \
ComposeSettingsForm.ui \
DisplaySettingsForm.ui \
IdentitySettingsForm.ui \
SendingSettingsForm.ui \
EmpathSettingsDialog.cpp \
EmpathSetupWizard.cpp

#EmpathFilterActionDialog.cpp \
#EmpathFilterEditDialog.cpp \
#EmpathFilterListItem.cpp \
#EmpathFilterManagerDialog.cpp \
#EmpathMatchPropertiesDialog.cpp \
#EmpathAccountsSettingsDialog.cpp \
#EmpathConfigIMAP4Dialog.cpp \
#EmpathConfigMaildirDialog.cpp \
#EmpathConfigPOP3Widget.cpp \
#EmpathConfigPOP3Dialog.cpp \
#EmpathConfigPOP3Logging.cpp \
#EmpathConfigPOP3Server.cpp \
#EmpathPasswordEditWidget.cpp \
#EmpathPathSelectWidget.cpp \
